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

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 695;

float alpha = 0.5f;
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
unsigned int loadCubemap(std::vector<std::string> faces);

bool cubeActive1 = false;
bool cubeActive2 = false;

bool kernelBool = false;
bool grayscaleBool = false;
bool nightVision = false;
bool inverseBool = false;

bool reflectionBool = false;
bool refractionBool = false;

int main()
{
	GLFWwindow* window = InitWindow();
	if (!window)
		return -1;

	Shader shader("res/shaders/Basic.shader");
	Shader stencil("res/shaders/Stencil.shader");
	Shader screen("res/shaders/Screen.shader");
	Shader skybox("res/shaders/Skybox.shader");
	Shader reflection("res/shaders/Reflection.shader");

	float cubeVertices[] = {
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

	float normalVertices[] = {
		// Positions			 normals
		-0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,	 0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,	 0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	 0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	 0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	 0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	 0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,	 0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,	 0.0f,  1.0f,  0.0f
	};

	float planeVertices[] = {
		// positions			 texture coords
		 5.0f, -0.5f,  5.0f,	 4.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,	 0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,	 0.0f, 4.0f,
								 
		 5.0f, -0.5f,  5.0f,	 4.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,	 0.0f, 4.0f,
		 5.0f, -0.5f, -5.0f,	 4.0f, 4.0f
	};

	float quadVertices[] = {
		// positions			 texCoords
		0.5f,  1.0f,			 0.0f, 1.0f,
		0.5f,  0.5f,			 0.0f, 0.0f,
		1.0f,  0.5f,			 1.0f, 0.0f,
								 
		0.5f,  1.0f,			 0.0f, 1.0f,
		1.0f,  0.5f,			 1.0f, 0.0f,
		1.0f,  1.0f,			 1.0f, 1.0f
	};

	// Car Mirror
	//-0.3f, 1.0f
	//- 0.3f, 0.7f
	//0.3f, 0.7f
	//
	//- 0.3f, 1.0f
	//0.3f, 0.7f
	//0.3f, 1.0f

	// Full Screen Framebuffer
	//-1.0f, 1.0f, 0.0f, 1.0f,
	//-1.0f, -1.0f, 0.0f, 0.0f,
	//1.0f, -1.0f, 1.0f, 0.0f,
	//
	//-1.0f, 1.0f, 0.0f, 1.0f,
	//1.0f, -1.0f, 1.0f, 0.0f,
	//1.0f, 1.0f, 1.0f, 1.0f

	float transparentVertices[] = {
		// positions			 texture Coords				
		0.0f,  0.5f,  0.0f,		 0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,		 0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,		 1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,		 0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,		 1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,		 1.0f,  0.0f
	};

	std::vector<glm::vec3> windows
	{
		glm::vec3(-1.5f, 0.0f, -0.48f),
		glm::vec3(1.5f, 0.0f, 0.51f),
		glm::vec3(0.0f, 0.0f, 0.7f),
		glm::vec3(-0.3f, 0.0f, -2.3f),
		glm::vec3(0.5f, 0.0f, -0.6f)
	};

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

	// Cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	glBindVertexArray(0);

	// Cube VAO (Normal)
	unsigned int reflectVAO, reflectVBO;
	glGenVertexArrays(1, &reflectVAO);
	glGenBuffers(1, &reflectVBO);

	glBindVertexArray(reflectVAO);

	glBindBuffer(GL_ARRAY_BUFFER, reflectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normalVertices), &normalVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	// Plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	// Transparent VAO
	unsigned int transparentVAO, transparentVBO;
	glGenVertexArrays(1, &transparentVAO);
	glGenBuffers(1, &transparentVBO);

	glBindVertexArray(transparentVAO);

	glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	// Quad VAO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	// CubeMap
	unsigned int cubemapVAO, cubemapVBO;
	glGenVertexArrays(1, &cubemapVAO);
	glGenBuffers(1, &cubemapVBO);

	glBindVertexArray(cubemapVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	std::vector<std::string> faces
	{
		"res/skyboxes/mountains/right.jpg",
		"res/skyboxes/mountains/left.jpg",
		"res/skyboxes/mountains/top.jpg",
		"res/skyboxes/mountains/bottom.jpg",
		"res/skyboxes/mountains/front.jpg",
		"res/skyboxes/mountains/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// Textures
	stbi_set_flip_vertically_on_load(true);
	unsigned int cubeTexture = loadTexture("res/textures/CrashBox.png");
	unsigned int floorTexture = loadTexture("res/textures/AztecStone.jpg");
	unsigned int wallTexture = loadTexture("res/textures/StoneWall.png");
	stbi_set_flip_vertically_on_load(false);
	unsigned int grassTexture = loadTexture("res/textures/grass.png");
	unsigned int windowTexture = loadTexture("res/textures/window.png");

	shader.Bind();
	shader.SetUniform1i("texture1", 0);
	screen.Bind();
	screen.SetUniform1i("screenTexture", 0);
	skybox.Bind();
	skybox.SetUniform1i("skybox", 0);
	reflection.Bind();
	reflection.SetUniform1i("skybox", 0);

	// Framebuffer
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	unsigned int TexColorBuffer;
	glGenTextures(1, &TexColorBuffer);
	glBindTexture(GL_TEXTURE_2D, TexColorBuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TexColorBuffer, 0);

	// Renderbuffer
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(0);

	// Game Loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		// Sort Windows
		std::map<float, glm::vec3> sorted;
		for (unsigned int i = 0; i < windows.size(); i++)
		{
			float distance = glm::length(camera.Position - windows[i]);
			sorted[distance] = windows[i];
		}

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glEnable(GL_DEPTH_TEST);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		/* FIRST ITERATION -> MIRROR */

		// Projections
		shader.Bind();
		
		glm::mat4 model = glm::mat4(1.0f);
		
		camera.Yaw += 180.0f;
		camera.Pitch = -camera.Pitch;
		camera.ProcessMouseMovement(0, 0, false);
		glm::mat4 view = camera.GetViewMatrix();
		
		camera.Yaw -= 180.0f;
		camera.Pitch = -camera.Pitch;
		camera.ProcessMouseMovement(0, 0, true);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		
		shader.SetUniformMatrix4fv("view", view);
		shader.SetUniformMatrix4fv("projection", projection);

		glDisable(GL_CULL_FACE);
		glStencilMask(0x00);

		// Floor
		glBindVertexArray(planeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		shader.SetUniformMatrix4fv("model", glm::mat4(1.0f));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Walls
		glBindTexture(GL_TEXTURE_2D, wallTexture);
		model = glm::translate(model, glm::vec3(-5.5f, -4.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6); // left
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(4.5f, -4.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6); // right
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -4.0f, 5.5f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6); // back
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -4.0f, -4.5f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6); // front
		glBindVertexArray(0);

		// Cube (Reflection/Refraction)
		reflection.Bind();
		
		model = glm::mat4(1.0f);

		camera.Yaw += 180.0f;
		camera.Pitch = -camera.Pitch;
		camera.ProcessMouseMovement(0, 0, false);
		view = camera.GetViewMatrix();
		
		camera.Yaw -= 180.0f;
		camera.Pitch = -camera.Pitch;
		camera.ProcessMouseMovement(0, 0, true);
		
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		reflection.SetUniformMatrix4fv("view", view);
		reflection.SetUniformMatrix4fv("projection", projection);
		reflection.SetUniform3f("cameraPos", camera.Position);

		glBindVertexArray(reflectVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		reflectionBool = true;
		if (reflectionBool)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-2.0f, 2.0f, -1.0f));
			reflection.SetUniformMatrix4fv("model", model);

			reflectionBool = false;
			reflection.SetUniform1i("reflectionBool", 1);
			reflection.SetUniform1i("refractionBool", 0);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		refractionBool = true;
		if (refractionBool)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(2.0f, 2.0f, -1.0f));
			reflection.SetUniformMatrix4fv("model", model);

			refractionBool = false;
			reflection.SetUniform1i("reflectionBool", 0);
			reflection.SetUniform1i("refractionBool", 1);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);

		shader.Bind();
		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		// Cubes
		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.01f, -1.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.01f, 0.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDisable(GL_CULL_FACE);
		glStencilMask(0x00);

		// Windows
		glBindVertexArray(transparentVAO);
		glBindTexture(GL_TEXTURE_2D, windowTexture);
		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			shader.SetUniformMatrix4fv("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		glBindVertexArray(0);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glDisable(GL_DEPTH_TEST);

		// Stencil Shader
		stencil.Bind();
		stencil.SetUniformMatrix4fv("view", view);
		stencil.SetUniformMatrix4fv("projection", projection);
		float scale = 1.1;

		// Stencil Cubes
		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, -0.01f, -1.0f));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencil.SetUniformMatrix4fv("model", model);
		if (cubeActive1)
			glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, -0.01f, 0.0f));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencil.SetUniformMatrix4fv("model", model);
		if (cubeActive2)
			glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);

		// Cubemap
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		skybox.Bind();

		camera.Yaw += 180.0f;
		camera.Pitch = -camera.Pitch;
		camera.ProcessMouseMovement(0, 0, false);
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

		camera.Yaw -= 180.0f;
		camera.Pitch = -camera.Pitch;
		camera.ProcessMouseMovement(0, 0, true);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		skybox.SetUniformMatrix4fv("view", view);
		skybox.SetUniformMatrix4fv("projection", projection);

		glBindVertexArray(cubemapVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);
		glDisable(GL_DEPTH_TEST);

		/* SECOND ITERATION -> NORMAL */

		// Framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Projections
		shader.Bind();
		model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		shader.SetUniformMatrix4fv("view", view);

		glDisable(GL_CULL_FACE);
		glStencilMask(0x00);

		// Floor
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		shader.SetUniformMatrix4fv("model", glm::mat4(1.0f));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Walls
		glBindTexture(GL_TEXTURE_2D, wallTexture);
		model = glm::translate(model, glm::vec3(-5.5f, -4.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6); // left
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(4.5f, -4.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6); // right
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -4.0f, 5.5f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6); // back
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -4.0f, -4.5f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6); // front
		glBindVertexArray(0);

		// Cube (Reflection/Refraction)
		reflection.Bind();

		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		reflection.SetUniformMatrix4fv("view", view);
		reflection.SetUniformMatrix4fv("projection", projection);
		reflection.SetUniform3f("cameraPos", camera.Position);

		glBindVertexArray(reflectVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		model = glm::translate(model, glm::vec3(-2.0f, 2.0f, -1.0f));
		reflection.SetUniformMatrix4fv("model", model);
		reflectionBool = true;
		if (reflectionBool)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-2.0f, 2.0f, -1.0f));
			reflection.SetUniformMatrix4fv("model", model);

			reflectionBool = false;
			reflection.SetUniform1i("reflectionBool", 1);
			reflection.SetUniform1i("refractionBool", 0);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		refractionBool = true;
		if (refractionBool)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(2.0f, 2.0f, -1.0f));
			reflection.SetUniformMatrix4fv("model", model);

			refractionBool = false;
			reflection.SetUniform1i("reflectionBool", 0);
			reflection.SetUniform1i("refractionBool", 1);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);

		shader.Bind();
		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		// Cubes
		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.01f, -1.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.01f, 0.0f));
		shader.SetUniformMatrix4fv("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDisable(GL_CULL_FACE);
		glStencilMask(0x00);

		// Windows
		glBindVertexArray(transparentVAO);
		glBindTexture(GL_TEXTURE_2D, windowTexture);
		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			shader.SetUniformMatrix4fv("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		glBindVertexArray(0);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glDisable(GL_DEPTH_TEST);

		// Stencil Shader
		stencil.Bind();
		stencil.SetUniformMatrix4fv("view", view);
		stencil.SetUniformMatrix4fv("projection", projection);

		// Stencil Cubes
		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, -0.01f, -1.0f));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencil.SetUniformMatrix4fv("model", model);
		if (cubeActive1)
			glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, -0.01f, 0.0f));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		stencil.SetUniformMatrix4fv("model", model);
		if (cubeActive2)
			glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);

		// Cubemap
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		skybox.Bind();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skybox.SetUniformMatrix4fv("view", view);
		skybox.SetUniformMatrix4fv("projection", projection);

		glBindVertexArray(cubemapVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);
		glDisable(GL_DEPTH_TEST);

		// Mirror
		screen.Bind();

		if (kernelBool)
		{
			screen.SetUniform1i("kernelBool", 1);
			screen.SetUniform1i("grayscaleBool", 0);
			screen.SetUniform1i("nightVision", 0);
			screen.SetUniform1i("inverseBool", 0);
		}
		else if (grayscaleBool)
		{
			screen.SetUniform1i("grayscaleBool", 1);
			if (nightVision)
				screen.SetUniform1i("nightVision", 1);
			screen.SetUniform1i("kernelBool", 0);
			screen.SetUniform1i("inverseBool", 0);
		}
		else if (inverseBool)
		{
			screen.SetUniform1i("kernelBool", 0);
			screen.SetUniform1i("grayscaleBool", 0);
			screen.SetUniform1i("nightVision", 0);
			screen.SetUniform1i("inverseBool", 1);
		}
		else
		{
			screen.SetUniform1i("kernelBool", 0);
			screen.SetUniform1i("grayscaleBool", 0);
			screen.SetUniform1i("nightVision", 0);
			screen.SetUniform1i("inverseBool", 0);
		}

		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, TexColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &reflectVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteVertexArrays(1, &transparentVAO);
	glDeleteVertexArrays(1, &cubemapVAO);

	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &reflectVBO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &transparentVBO);
	glDeleteBuffers(1, &cubemapVBO);

	glDeleteFramebuffers(1, &FBO);
	glDeleteRenderbuffers(1, &RBO);
	
	glDeleteTextures(1, &TexColorBuffer);

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
	glDepthFunc(GL_LESS);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

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

	// Cube Selection
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && !cubeActive1)
		cubeActive1 = true;
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && !cubeActive2)
		cubeActive2 = true;
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
	{
		cubeActive1 = false;
		cubeActive2 = false;
	}

	// Post-Processing
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		kernelBool = true;
		grayscaleBool = false;
		nightVision = false;
		inverseBool = false;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		kernelBool = false;
		grayscaleBool = true;
		nightVision = false;
		inverseBool = false;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		kernelBool = false;
		grayscaleBool = true;
		nightVision = true;
		inverseBool = false;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
	{
		kernelBool = false;
		grayscaleBool = false;
		nightVision = false;
		inverseBool = true;
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
	{
		kernelBool = false;
		grayscaleBool = false;
		nightVision = false;
		inverseBool = false;
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