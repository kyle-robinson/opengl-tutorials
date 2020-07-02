#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <irrKlang/irrKlang.h>
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw_gl3.h"

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

Camera camera(glm::vec3(0.0f, 0.0f, 155.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool mouseActive = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool blinn = false;
bool blinnKeyPressed = false;

GLFWwindow* InitWindow();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);

int main()
{
	GLFWwindow* window = InitWindow();
	if (!window)
		return -1;

	//Shader shader("res/shaders/Basic.shader");
	Shader planetShader("res/shaders/Planet.shader");
	Shader asteroidShader("res/shaders/Asteroid.shader");
	Shader skyboxShader("res/shaders/Skybox.shader");

	Model mars("res/models/mars/planet.obj");
	Model earth("res/models/earth/Earth.obj");
	Model rock("res/models/asteroid/rock.obj");

	unsigned int amount = 100000;
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(glfwGetTime());
	float radius = 150.0;
	float offset = 25.0f;
	for (unsigned int i = 0; i < amount; i++)
	{
		// 1. Translation - Displace along circle in range [-offset, offset]
		glm::mat4 model = glm::mat4(1.0f);
		float angle = (float)i / (float)amount * 360.0f;
		float x;
		float z;

		// X
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		if (i % 3 && (i % 3) != (i % 2))
			x = sin(angle) * radius + displacement;
		else if (i % 2)
			x = sin(angle) * radius * 1.5f + displacement;
		else
			x = sin(angle) * radius * 2.0f + displacement;

		// Y
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.2f;

		// Z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		if (i % 3 && (i % 3) != (i % 2))
			z = cos(angle) * radius + displacement;
		else if (i % 2)
			z = cos(angle) * radius * 1.5f + displacement;
		else
			z = cos(angle) * radius * 2.0f + displacement;
		
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. Scale - Scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. Rotation - Added random rotation around randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. Add to list of matrices
		modelMatrices[i] = model;
	}

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	for (unsigned int i = 0; i < rock.meshes.size(); i++)
	{
		unsigned int VAO = rock.meshes[i].VAO;
		glBindVertexArray(VAO);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));

		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}

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

	unsigned int cubemapVAO, cubemapVBO;
	glGenVertexArrays(1, &cubemapVAO);
	glGenBuffers(1, &cubemapVBO);

	glBindVertexArray(cubemapVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	std::vector<std::string> spaceRed
	{
		"res/skyboxes/space/red/right.png",
		"res/skyboxes/space/red/left.png",
		"res/skyboxes/space/red/top.png",
		"res/skyboxes/space/red/bottom.png",
		"res/skyboxes/space/red/front.png",
		"res/skyboxes/space/red/back.png"
	};
	std::vector<std::string> spaceCyan
	{
		"res/skyboxes/space/lightblue/right.png",
		"res/skyboxes/space/lightblue/left.png",
		"res/skyboxes/space/lightblue/top.png",
		"res/skyboxes/space/lightblue/bottom.png",
		"res/skyboxes/space/lightblue/front.png",
		"res/skyboxes/space/lightblue/back.png"
	};
	std::vector<std::string> spaceBlue
	{
		"res/skyboxes/space/blue/right.png",
		"res/skyboxes/space/blue/left.png",
		"res/skyboxes/space/blue/top.png",
		"res/skyboxes/space/blue/bottom.png",
		"res/skyboxes/space/blue/front.png",
		"res/skyboxes/space/blue/back.png"
	};
	unsigned int cubemapRedTexture = loadCubemap(spaceRed);
	unsigned int cubemapCyanTexture = loadCubemap(spaceCyan);
	unsigned int cubemapBlueTexture = loadCubemap(spaceBlue);

	bool spaceRed_Bool = false;
	bool spaceCyan_Bool = false;
	bool spaceBlue_Bool = true;

	skyboxShader.Bind();
	skyboxShader.SetUniform1i("skybox", 0);

	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();
	glm::vec3 marsTranslation(0.0f, -3.0f, 0.0f);
	glm::vec3 earthTranslation(500.0f, -3.0f, 100.0f);

	// Coloured Quads
	/*glm::vec2 translations[100];
	int index = 0;
	float offset = 0.1f;
	for (int y = -10; y < 10; y += 2)
	{
		for (int x = -10; x < 10; x += 2)
		{
			glm::vec2 translation;
			translation.x = (float)x / 10.0f + offset;
			translation.y = (float)y / 10.0f + offset;
			translations[index++] = translation;
		}
	}

	unsigned int instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	float quadVertices[] = {
		// positions     // colors
		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		-0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		 0.05f,  0.05f,  0.0f, 1.0f, 1.0f
	};

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(2, 1);

	glBindVertexArray(0);*/

	glUseProgram(0);

	// Game Loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplGlfwGL3_NewFrame();

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		float rotAngle = glfwGetTime() * 0.2f;

		// Setup Planet
		planetShader.Bind();
		planetShader.SetUniform3f("viewPos", camera.Position);

		planetShader.SetUniform3f("dirLight.direction", -0.2f, -1.0f, -0.3f);
		planetShader.SetUniform3f("dirLight.ambient", 0.0f, 0.0f, 0.0f);
		planetShader.SetUniform3f("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		planetShader.SetUniform3f("dirLight.specular", 0.2f, 0.2f, 0.2f);

		planetShader.SetUniform1i("blinn", blinn);
		planetShader.SetUniform1f("material.shininess", 64.0f);

		planetShader.SetUniformMatrix4fv("projection", projection);
		planetShader.SetUniformMatrix4fv("view", view);

		// Planets
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(marsTranslation));
		model = glm::rotate(model, rotAngle * 0.2f, glm::vec3(0.0f, glfwGetTime(), 0.0f));
		model = glm::scale(model, glm::vec3(16.0f, 16.0f, 16.0f));
		planetShader.SetUniformMatrix4fv("model", model);
		mars.Draw(planetShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, earthTranslation);
		model = glm::rotate(model, rotAngle * 0.2f, glm::vec3(0.0f, glfwGetTime(), 0.0f));
		model = glm::scale(model, glm::vec3(16.0f, 16.0f, 16.0f));
		planetShader.SetUniformMatrix4fv("model", model);
		earth.Draw(planetShader);
		
		// Setup Asteroids
		asteroidShader.Bind();
		asteroidShader.SetUniform3f("viewPos", camera.Position);

		asteroidShader.SetUniform3f("dirLight.direction", -0.2f, -1.0f, -0.3f);
		asteroidShader.SetUniform3f("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		asteroidShader.SetUniform3f("dirLight.diffuse", 0.2f, 0.2f, 0.2f);
		asteroidShader.SetUniform3f("dirLight.specular", 0.1f, 0.1f, 0.1f);

		asteroidShader.SetUniform1i("blinn", blinn);
		asteroidShader.SetUniform1f("material.shininess", 64.0f);

		asteroidShader.SetUniformMatrix4fv("projection", projection);
		asteroidShader.SetUniformMatrix4fv("view", view);

		// Rotate Asteroids
		model = glm::mat4(1.0f);
		model = glm::rotate(model, rotAngle, glm::vec3(glfwGetTime() * 0.1f, glfwGetTime() * 0.3f, 0.0f));
		model = glm::translate(model, glm::vec3(rotAngle, -3.0f, 0.0f));
		asteroidShader.SetUniformMatrix4fv("model", model);

		// Asteroids
		asteroidShader.SetUniform1i("material.texture_diffuse1", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
		for (unsigned int i = 0; i < rock.meshes.size(); i++)
		{
			glBindVertexArray(rock.meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, rock.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
			glBindVertexArray(0);
		}

		// Cubemap
		glDepthFunc(GL_LEQUAL);

		skyboxShader.Bind();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.SetUniformMatrix4fv("view", view);
		skyboxShader.SetUniformMatrix4fv("projection", projection);

		glBindVertexArray(cubemapVAO);
		if (spaceRed_Bool)
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapRedTexture);
		else if (spaceCyan_Bool)
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapCyanTexture);
		else if (spaceBlue_Bool)
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapBlueTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);

		// ImGui Window
		ImGui::Begin("Main Window");
		{
			ImGui::Text("Mars Translation");
			ImGui::SliderFloat3("Mars", &marsTranslation.x, -500.0f, 500.0f);
			
			ImGui::Text("Earth Translation");
			ImGui::SliderFloat3("Earth", &earthTranslation.x, -500.0f, 500.0f);

			static int skyboxInt = 0;
			ImGui::Text("Skybox");
			if (ImGui::RadioButton("Red", &skyboxInt, 0))
			{
				spaceRed_Bool = true;
				spaceCyan_Bool = false;
				spaceBlue_Bool = false;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Cyan", &skyboxInt, 1))
			{
				spaceRed_Bool = false;
				spaceCyan_Bool = true;
				spaceBlue_Bool = false;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Blue", &skyboxInt, 2))
			{
				spaceRed_Bool = false;
				spaceCyan_Bool = false;
				spaceBlue_Bool = true;
			}

			static int lightInt = 0;
			ImGui::Text("Specular Model");
			if (ImGui::RadioButton("Phong", &lightInt, 0))
				blinn = false;
			ImGui::SameLine();
			if (ImGui::RadioButton("Blinn", &lightInt, 1))
				blinn = true;

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
		ImGui::End();

		// Coloured Quads
		/*shader.Bind();
		glBindVertexArray(quadVAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
		glBindVertexArray(0);*/

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &cubemapVAO);
	
	glDeleteBuffers(1, &cubemapVBO);
	glDeleteBuffers(1, &buffer);
	
	glDeleteTextures(1, &cubemapRedTexture);
	glDeleteTextures(1, &cubemapCyanTexture);
	glDeleteTextures(1, &cubemapBlueTexture);

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
	glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLFW Project", glfwGetPrimaryMonitor(), NULL);
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

	std::cout << "Using GL Version: " << glGetString(GL_VERSION) << std::endl << std::endl;
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();
	irrklang::ISound* Music = SoundEngine->play2D("res/audio/space.mp3", true, false, true);

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
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// Polygon Mode
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

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

	// Blinn-Phong
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
	{
		blinn = !blinn;
		blinnKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
	{
		blinnKeyPressed = false;
	}
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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