#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Camera.h"
#include "stb_image.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>
#include <map>

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;
const unsigned int SAMPLES = 8;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.5f, 3.0f));
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

GLFWwindow* InitWindow();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);

int main()
{
	GLFWwindow* window = InitWindow();
	if (!window)
		return -1;

	Shader redShader("res/shaders/Red.shader");
	Shader greenShader("res/shaders/Green.shader");
	Shader blueShader("res/shaders/Blue.shader");
	Shader yellowShader("res/shaders/Yellow.shader");

	float cubeVertices[] = {
		// positions
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
	};

	float texCubeVertices[] = {
		// positions			 texture coords
		// back face
		-0.5f, -0.5f, -0.5f,	 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,	 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	 0.0f, 0.0f,
		// front face
		-0.5f, -0.5f,  0.5f,	 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	 0.0f, 1.0f,
		// left face
		-0.5f,  0.5f,  0.5f,	 1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	 0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	 0.0f, 0.0f,
		// right face
		 0.5f,  0.5f,  0.5f,	 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	 1.0f, 0.0f,
		 // bottom face
		 -0.5f, -0.5f, -0.5f,	 0.0f, 1.0f,
		  0.5f, -0.5f,  0.5f,	 1.0f, 0.0f,
		  0.5f, -0.5f, -0.5f,	 1.0f, 1.0f,
		  0.5f, -0.5f,  0.5f,	 1.0f, 0.0f,
		 -0.5f, -0.5f, -0.5f,	 0.0f, 1.0f,
		 -0.5f, -0.5f,  0.5f,	 0.0f, 0.0f,
		 // top face
		 -0.5f,  0.5f, -0.5f,	 0.0f, 1.0f,
		  0.5f,  0.5f, -0.5f,	 1.0f, 1.0f,
		  0.5f,  0.5f,  0.5f,	 1.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,	 1.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f,	 0.0f, 0.0f,
		 -0.5f,  0.5f, -0.5f,	 0.0f, 1.0f
	};

	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	unsigned int texCubeVAO, texCubeVBO;
	glGenVertexArrays(1, &texCubeVAO);
	glGenBuffers(1, &texCubeVBO);

	glBindVertexArray(texCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, texCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCubeVertices), &texCubeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	// Textures
	unsigned int frontTexture = loadTexture("res/textures/container.png");
	unsigned int backTexture = loadTexture("res/textures/CrashBox.png");

	greenShader.Bind();
	greenShader.SetUniform1i("frontTexture", 0);
	greenShader.SetUniform1i("backTexture", 1);
	greenShader.UnBind();

	// Uniform Block
	unsigned int uniformBlockIndexRed = glGetUniformBlockIndex(redShader.GetID(), "Matrices");
	unsigned int uniformBlockIndexGreen = glGetUniformBlockIndex(greenShader.GetID(), "Matrices");
	unsigned int uniformBlockIndexBlue = glGetUniformBlockIndex(blueShader.GetID(), "Matrices");
	unsigned int uniformBlockIndexYellow = glGetUniformBlockIndex(yellowShader.GetID(), "Matrices");

	glUniformBlockBinding(redShader.GetID(), uniformBlockIndexRed, 0);
	glUniformBlockBinding(greenShader.GetID(), uniformBlockIndexGreen, 0);
	glUniformBlockBinding(blueShader.GetID(), uniformBlockIndexBlue, 0);
	glUniformBlockBinding(yellowShader.GetID(), uniformBlockIndexYellow, 0);

	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));

	glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUseProgram(0);

	// Game Loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glm::mat4 view = camera.GetViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		redShader.Bind();
		glm::mat4 model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f));
		redShader.SetUniformMatrix4fv("model", model);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		greenShader.Bind();
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f));
		greenShader.SetUniformMatrix4fv("model", model);
		glBindVertexArray(texCubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frontTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, backTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		blueShader.Bind();
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f));
		blueShader.SetUniformMatrix4fv("model", model);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		yellowShader.Bind();
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f));
		yellowShader.SetUniformMatrix4fv("model", model);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

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
	glfwWindowHint(GLFW_SAMPLES, SAMPLES);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLFW Project", NULL, NULL);
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
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_MULTISAMPLE);

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

	// Polygon Mode
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
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
		//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, SAMPLES, format, width, height, GL_TRUE);
		glGenerateMipmap(GL_TEXTURE_2D);

		if (format == GL_RGBA)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
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