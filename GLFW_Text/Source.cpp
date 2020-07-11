#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "stb_image.h"

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw_gl3.h"

#include "Shader.h"
#include "Camera.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>
#include <map>

// Globals
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

unsigned int quadVAO = 0, quadVBO;
unsigned int cubeVAO = 0, cubeVBO;
unsigned int skyVAO = 0, skyVBO;

// Camera
Camera camera(glm::vec3(0.0f, 0.5f, 3.0f));
bool firstMouse = true;
bool mouseActive = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Text
struct Character
{
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int Advance;
};
std::map<char, Character> Characters;
unsigned int VAO, VBO;

std::string matrixString = "Welcome to the Matrix!";
glm::vec2 matrixPos = { (float)SCR_WIDTH / 3, ((float)SCR_HEIGHT / 2) };
glm::vec3 matrixColor = { 0.0f, 1.0f, 0.0f };
bool enableBlend = true;
bool blendKeyPressed = false;

// Prototypes
GLFWwindow* InitWindow();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path, bool gammaCorrection);
unsigned int loadCubemap(std::vector<std::string> faces);
void renderQuad();
void renderCube();
void renderCubemap();
void RenderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color);
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

int main()
{
	GLFWwindow* window = InitWindow();
	if (!window)
		return -1;

	Shader basic("res/shaders/Basic.shader");
	Shader skybox("res/shaders/Skybox.shader");
	Shader shader("res/shaders/Text.shader");

	// a value different than 0 is returned whan an error occurs
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library!" << std::endl;
		return -1;
	}

	// load font as face
	FT_Face face;
	if (FT_New_Face(ft, "res/fonts/VT323-Regular.ttf", 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font!" << std::endl;
		return -1;
	}
	else
	{
		FT_Set_Pixel_Sizes(face, 0, 48); // set size for glyphs to load
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

		// load first 128 chars of ASCII set
		for (unsigned char c = 0; c < 128; c++)
		{
			// load glyph
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYPE: Failed to load Glyph!" << std::endl;
				continue;
			}

			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0,  GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// store character for later use
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			Characters.insert(std::pair<char, Character>(c, character));
		}
	}
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup textures
	basic.Bind();
	unsigned int boxTexture = loadTexture("res/textures/Matrix.jpg", true);
	basic.SetUniform1i("diffuseMap", 0);

	std::vector<std::string> matrixVector = {
		"res/skyboxes/matrix/right.jpg",
		"res/skyboxes/matrix/left.jpg",
		"res/skyboxes/matrix/top.jpg",
		"res/skyboxes/matrix/bottom.jpg",
		"res/skyboxes/matrix/front.jpg",
		"res/skyboxes/matrix/back.jpg"
	};

	skybox.Bind();
	unsigned int matrixCubemap = loadCubemap(matrixVector);
	skybox.SetUniform1i("skybox", 0);

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	// Game Loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		ImGui_ImplGlfwGL3_NewFrame();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render Plane
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		basic.Bind();
		basic.SetUniformMatrix4fv("projection", projection);
		basic.SetUniformMatrix4fv("view", view);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, boxTexture);
		
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
		basic.SetUniformMatrix4fv("model", model);
		renderCube();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.5f, 0.0f, 2.5f));
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * 2.5f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
		basic.SetUniformMatrix4fv("model", model);
		renderCube();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.5f, 0.0f, 2.5f));
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * 2.5f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
		basic.SetUniformMatrix4fv("model", model);
		renderCube();
		
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.0f));
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * 2.5f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
		basic.SetUniformMatrix4fv("model", model);
		renderCube();
		
		// Text Rendering
		shader.Bind();
		projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
		shader.SetUniformMatrix4fv("projection", projection);

		RenderText(shader, matrixString, matrixPos.x, matrixPos.y, 1.0f, matrixColor);

		// Render Skybox
		glDepthFunc(GL_LEQUAL);
		skybox.Bind();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skybox.SetUniformMatrix4fv("projection", projection);
		skybox.SetUniformMatrix4fv("view", view);
		
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * 2.5f), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
		skybox.SetUniformMatrix4fv("model", model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, matrixCubemap);
		renderCubemap();

		glDepthFunc(GL_LESS);

		// ImGui Window
		ImGui::Begin("Main Window", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		{
			if (ImGui::CollapsingHeader("Text Rendering"))
			{
				char* cstr = &matrixString[0];
				ImGui::InputText("String", cstr, 30); // anything more than 30 causes heap corruption
				matrixString = cstr;

				ImGui::SliderFloat2("Position", &matrixPos.x, 0.0f, 1000.0f, "%1.f");

				ImGui::SliderFloat3("Colour", &matrixColor.x, 0.0f, 1.0f, "%.1f");

				static int e = 0;
				ImGui::Text("Blend");// ImGui::SameLine();
				
				if (ImGui::RadioButton("Enable", &e, 0))
					enableBlend = true; ImGui::SameLine();

				if (ImGui::RadioButton("Disable", &e, 1))
					enableBlend = false;

				enableBlend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
			}

			if (ImGui::CollapsingHeader("Application Info"))
			{
				ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
				ImGui::Text("Shader Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
				ImGui::Text("Hardware: %s", glGetString(GL_RENDERER));
				ImGui::NewLine();
				ImGui::Text("Frametime: %.3f / Framerate: (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}

			if (ImGui::CollapsingHeader("About"))
			{
				ImGui::Text("Text Rendering Demo by Kyle Robinson");
				ImGui::NewLine();
				ImGui::Text("Email: kylerobinson456@outlook.com");
				ImGui::Text("Twitter: @KyleRobinson42");
			}
		}
		ImGui::End();
		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteVertexArrays(1, &VAO);

	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &VBO);

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

GLFWwindow* InitWindow()
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialise GLFW!" << std::endl;
		return nullptr;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Text Rendering", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowPos(window, 100, 100);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD." << std::endl;
		return nullptr;
	}

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	std::cout << "Using GL Version: " << glGetString(GL_VERSION) << std::endl << std::endl;
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	if (mouseActive)
		camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	// Camera Movement
	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// Mouse Control
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		mouseActive = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		mouseActive = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	// Blending
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !blendKeyPressed)
	{
		enableBlend = !enableBlend;
		blendKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE)
	{
		blendKeyPressed = false;
	}
}

unsigned int loadTexture(char const* path, bool gammaCorrection)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum dataFormat;
		GLenum internalFormat;
		if (nrChannels == 1)
			dataFormat = internalFormat = GL_RED;
		else if (nrChannels == 3)
		{
			dataFormat = GL_RGB;
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
		}
		else if (nrChannels == 4)
		{
			dataFormat = GL_RGBA;
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);

		glBindVertexArray(quadVAO);

		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void renderCube()
{
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
		   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, // bottom-left
			0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // top-right
			0.5f, -0.5f, -0.5f,  1.0f,  0.0f, // bottom-right         
			0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // top-right
		   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, // bottom-left
		   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f, // top-left
			// front face
		   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-left
			0.5f, -0.5f,  0.5f,  1.0f,  0.0f, // bottom-right
			0.5f,  0.5f,  0.5f,  1.0f,  1.0f, // top-right
			0.5f,  0.5f,  0.5f,  1.0f,  1.0f, // top-right
		   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f, // top-left
		   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-left
			// left face
		   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f, // top-right
		   -0.5f,  0.5f, -0.5f, -1.0f,  1.0f, // top-left
		   -0.5f, -0.5f, -0.5f, -0.0f,  1.0f, // bottom-left
		   -0.5f, -0.5f, -0.5f, -0.0f,  1.0f, // bottom-left
		   -0.5f, -0.5f,  0.5f, -0.0f,  0.0f, // bottom-right
		   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f, // top-right
			// right face
			0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // top-left
			0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // bottom-right
			0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // top-right         
			0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // bottom-right
			0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // top-left
			0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-left     
			// bottom face
		   -0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // top-right
			0.5f, -0.5f, -0.5f,  1.0f,  1.0f, // top-left
			0.5f, -0.5f,  0.5f,  1.0f,  0.0f, // bottom-left
			0.5f, -0.5f,  0.5f,  1.0f,  0.0f, // bottom-left
		   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // bottom-right
		   -0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // top-right
			// top face
		   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f, // top-left
			0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // bottom-right
			0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // top-right     
			0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // bottom-right
		   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f, // top-left
		   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f  // bottom-left        
		};

		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);

		glBindVertexArray(cubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void renderCubemap()
{
	if (skyVAO == 0)
	{
		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		glGenVertexArrays(1, &skyVAO);
		glGenBuffers(1, &skyVBO);

		glBindVertexArray(skyVAO);

		glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(skyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void RenderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color)
{
	s.Bind();
	s.SetUniform3f("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// update VAO for each character
		float  vertices[6][4] = {
			{ xpos, ypos + h, 0.0f, 0.0f },
			{ xpos, ypos, 0.0f, 1.0f },
			{ xpos + w, ypos, 1.0f, 1.0f },

			{ xpos, ypos + h, 0.0f, 0.0f },
			{ xpos + w, ypos, 1.0f, 1.0f },
			{ xpos + w, ypos + h, 1.0f, 0.0f }
		};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID); // render glyph texture over quad

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void APIENTRY glDebugOutput(GLenum source,
							GLenum type,
							unsigned int id,
							GLenum severity,
							GLsizei length,
							const char* message,
							const void* userParam)
{
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API: std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER: std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER: std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER: std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: Medium"; break;
	case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}