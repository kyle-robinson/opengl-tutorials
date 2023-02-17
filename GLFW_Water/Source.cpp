#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <irrKlang/irrKlang.h>

#include "stb_image.h"
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw_gl3.h"

#include "Shader.h"
#include "Camera.h"
#include <iostream>

// Globals
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

unsigned int skyVAO = 0, skyVBO;
float speedF = 0.8f;
float amountF = 0.01f;
float heightF = 0.5f;
float multiplier = 0.5f;
bool lightingBool = true;
bool isLava = false;
bool bothTextures = false;

// Audio
irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();
irrklang::ISound* ocean = SoundEngine->play2D("res/audio/ocean.mp3", true, true, false);
irrklang::ISound* lava = SoundEngine->play2D("res/audio/lava.mp3", true, true, false);
bool waterIsPlaying = false;
bool lavaIsPlaying = false;

// Camera
Camera camera(glm::vec3(0.0f, 0.5f, 3.0f));
bool firstMouse = true;
bool mouseActive = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Prototypes
GLFWwindow* InitWindow();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

unsigned int loadTexture(char const* path, bool gammaCorrection);
unsigned int loadCubemap(std::vector<std::string> faces);
void renderCubemap();
std::vector<float>* loadHeightMap(unsigned char* data, float width, float height, float chan, float rec_width);
std::vector<unsigned int>* loadIndices(int size);
unsigned int loadObject(std::vector<float>* vertices, std::vector<unsigned int>* indices);
std::vector<float>* loadPlane(int size, float width, float height);
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

int main()
{
	GLFWwindow* window = InitWindow();
	if (!window)
		return -1;

	// Initialise cubemap
	Shader skybox("res/shaders/Skybox.shader");
	skybox.SetUniform1i("skybox", 0);
	skybox.SetUniform1i("hellbox", 1);
	std::vector<std::string> skyVector = {
		"res/skyboxes/clouds/blue/right.jpg",
		"res/skyboxes/clouds/blue/left.jpg",
		"res/skyboxes/clouds/blue/up.jpg",
		"res/skyboxes/clouds/blue/down.jpg",
		"res/skyboxes/clouds/blue/front.jpg",
		"res/skyboxes/clouds/blue/back.jpg"
	};
	std::vector<std::string> hotVector = {
		"res/skyboxes/hot/right.png",
		"res/skyboxes/hot/left.png",
		"res/skyboxes/hot/up.png",
		"res/skyboxes/hot/down.png",
		"res/skyboxes/hot/front.png",
		"res/skyboxes/hot/back.png"
	};
	skybox.Bind();
	unsigned int skyCubemap = loadCubemap(skyVector);
	unsigned int hotCubemap = loadCubemap(hotVector);

	// Initialise sand
	Shader sand("res/shaders/Sand.shader");
	sand.SetUniform1i("sandTexture", 2);
	unsigned int sandTexture = loadTexture("res/textures/ground.jpg", true);
	
	int width, height, nrChannels;
	float rec_width = 1.0f;
	
	unsigned char* data = stbi_load("res/textures/terrain.jpg", &width, &height, &nrChannels, 0);
	auto sandVertices = loadHeightMap(data, width, height, nrChannels, rec_width);
	stbi_image_free(data);

	auto sandIndices = loadIndices(width);
	unsigned int sandVAO = loadObject(sandVertices, sandIndices);

	// Initialise water
	Shader water("res/shaders/Water.shader");
	water.SetUniform1i("waterTexture", 3);
	water.SetUniform1i("skybox", 0);
	water.SetUniform1i("lavaTexture", 4);
	water.SetUniform1i("emissionTexture", 5);
	unsigned int waterTexture = loadTexture("res/textures/water.png", true);
	unsigned int lavaTexture = loadTexture("res/textures/lava/albedo.png", true);
	unsigned int emissionTexture = loadTexture("res/textures/lava/emission.png", true);
	auto waterVertices = loadPlane(width, rec_width, 4.0f);
	auto waterIndices = loadIndices(width);
	unsigned int waterVAO = loadObject(waterVertices, waterIndices);

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

		if (isLava && bothTextures)
		{
			bothTextures = false;
			isLava = false;
		}

		if (!bothTextures)
		{
			if (!isLava && !waterIsPlaying)
			{
				ocean->setIsPaused(false);
				lava->setIsPaused(true);
			}
			if (isLava && !lavaIsPlaying)
			{
				ocean->setIsPaused(true);
				lava->setIsPaused(false);
			}
		}
		else
		{
			ocean->setIsPaused(false);
			lava->setIsPaused(false);
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, hotCubemap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, sandTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, waterTexture);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, lavaTexture);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, emissionTexture);
		
		// Render Sand
		sand.Bind();
		sand.SetUniformMatrix4fv("projection", projection);
		sand.SetUniformMatrix4fv("view", view);
		sand.SetUniform1i("lighting", lightingBool);
		sand.SetUniform1i("isLava", isLava);
		sand.SetUniform1i("bothTextures", bothTextures);
		sand.SetUniform1i("viewportWidth", SCR_WIDTH);
		sand.SetUniform1f("viewportMultiplier", multiplier);
		
		glBindVertexArray(sandVAO);
		glDrawElements(GL_TRIANGLES, sandIndices->size(), GL_UNSIGNED_INT, 0);

		// Render Water
		water.Bind();
		water.SetUniformMatrix4fv("projection", projection);
		water.SetUniformMatrix4fv("view", view);

		water.SetUniform3f("cameraPos", camera.Position);
		water.SetUniform1i("lighting", lightingBool);
		water.SetUniform1i("isLava", isLava);
		water.SetUniform1i("bothTextures", bothTextures);
		water.SetUniform1i("viewportWidth", SCR_WIDTH);
		water.SetUniform1f("viewportMultiplier", multiplier);

		water.SetUniform1f("time", glfwGetTime());
		water.SetUniform1f("speed", speedF);
		water.SetUniform1f("amount", amountF);
		water.SetUniform1f("height", heightF);

		glBindVertexArray(waterVAO);
		glDrawElements(GL_TRIANGLES, waterIndices->size(), GL_UNSIGNED_INT, 0);

		// Render Skybox
		glDepthFunc(GL_LEQUAL);
		
		skybox.Bind();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		
		skybox.SetUniformMatrix4fv("projection", projection);
		skybox.SetUniformMatrix4fv("view", view);
		skybox.SetUniform1i("isLava", isLava);
		skybox.SetUniform1i("bothTextures", bothTextures);
		skybox.SetUniform1i("viewportWidth", SCR_WIDTH);
		skybox.SetUniform1f("viewportMultiplier", multiplier);

		renderCubemap();

		glDepthFunc(GL_LESS);

		// ImGui Window
		ImGui::Begin("Main Window", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		{
			if (ImGui::CollapsingHeader("Water Parameters"))
			{
				ImGui::SliderFloat("Speed", &speedF, 0.0f, 1.0f, "%.1f");
				ImGui::SliderFloat("Height", &heightF, 0.0f, 1.0f, "%.1f");

				static bool waveBool = true;
				ImGui::Checkbox("Waves", &waveBool);
				if (waveBool)
					amountF = 0.01f;
				else
					amountF = 0.0f;
			}

			if (ImGui::CollapsingHeader("Scene Parameters"))
			{
				ImGui::Checkbox("Hell Scene", &isLava);
				if (isLava)
					isLava = true;
				else
					isLava = false;

				ImGui::Checkbox("Water & Lava", &bothTextures);
				if (bothTextures)
					bothTextures = true;
				else
					bothTextures = false;

				ImGui::SliderFloat("Split Width", &multiplier, 0.0f, 1.0f, "%.1f");
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
				ImGui::Text("Water Simulation by Kyle Robinson");
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

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Water Simulation", NULL, NULL);
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

	// Scene Lighting
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		lightingBool = true;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		lightingBool = false;
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

std::vector<float>* loadHeightMap(unsigned char* data, float width, float height, float chan, float rec_width)
{
	auto vertices = new std::vector<float>();
	float start = -(width / 2) * rec_width;

	for (int x = 0; x < width; x++)
	{
		float pos_x = start + x * rec_width * 2;
		for (int z = 0; z < width; z++)
		{
			float pos_z = -(start + z * rec_width * 2);
			int pos_img = (x + z * width) * chan;
			float pixel = data[pos_img];

			// Positions
			pixel = (pixel / 10) - 10;
			vertices->push_back(pos_x);
			vertices->push_back(pixel);
			vertices->push_back(pos_z);

			// TexCoords
			vertices->push_back(pos_x);
			vertices->push_back(pos_z);
		}
	}
	return vertices;
}

std::vector<unsigned int>* loadIndices(int size)
{
	auto indices = new std::vector<unsigned int>();

	for (int z = 0; z < size - 1; ++z)
	{
		for (int x = 0; x < size - 1; ++x)
		{
			int start = x + z * size;
			indices->push_back(start);
			indices->push_back(start + 1);
			indices->push_back(start + size);
			indices->push_back(start + 1);
			indices->push_back(start + 1 + size);
			indices->push_back(start + size);
		}
	}
	return indices;
}

unsigned int loadObject(std::vector<float>* vertices, std::vector<unsigned int>* indices)
{
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(float), &vertices->at(0), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(unsigned int), &indices->at(0), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	return VAO;
}

std::vector<float>* loadPlane(int size, float width, float height)
{
	auto vertices = new std::vector<float>();
	float start = -(size / 2) * width;

	for (int x = 0; x < size; x++)
	{
		float pos_x = start + x * width * 2;
		for (int z = 0; z < size; z++)
		{
			// Positions
			float pos_z = -(start + z * width * 2);
			vertices->push_back(pos_x);
			vertices->push_back(height);
			vertices->push_back(pos_z);

			// TexCoords
			vertices->push_back(pos_x);
			vertices->push_back(pos_z);
		}
	}
	return vertices;
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