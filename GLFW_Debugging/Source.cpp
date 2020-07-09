#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw_gl3.h"

#include "Shader.h"
#include "Camera.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>

#define glCheckError() glCheckError_(__FILE__, __LINE__)

// Globals
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

unsigned int quadVAO = 0, quadVBO;
unsigned int cubeVAO = 0, cubeVBO;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool mouseActive = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Prototypes
GLFWwindow* InitWindow();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

unsigned int loadTexture(const char* path, bool gammaCorrection);
void renderQuad();
void renderCube();

GLenum glCheckError_(const char* file, int line);
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

int main()
{
    GLFWwindow* window = InitWindow();
    if (!window)
        return -1;

    Shader shader("res/shaders/Debugging.shader");

    unsigned int texture = loadTexture("res/textures/CrashBox.png", true);
    shader.SetUniform1i("diffuseMap", 0);

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

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        shader.Bind();
        shader.SetUniformMatrix4fv("projection", projection);
        shader.SetUniformMatrix4fv("view", view);

        float rotationSpeed = 10.0f;
        float angle = (float)glfwGetTime() * rotationSpeed;
        model = glm::translate(model, glm::vec3(0.0, 0.0f, -2.5));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 1.0f));
        shader.SetUniformMatrix4fv("model", model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        renderCube();

        // ImGui Window
        ImGui::Begin("Main Window", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        {
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
                ImGui::Text("OpenGL Debugging Demo by Kyle Robinson");
                ImGui::NewLine();
                ImGui::Text("Email: kylerobinson456@outlook.com");
                ImGui::Text("Twitter: @KyleRobinson42");
            }
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1, &cubeVAO);

    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &cubeVBO);

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Debugging", NULL, NULL);
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

    // Enalbe OpenGL debug context - only works for version 4.3 or higher
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
}

unsigned int loadTexture(const char* path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum dataFormat;
        GLenum internalFormat;
        if (nrComponents == 1)
            dataFormat = internalFormat = GL_RED;
        else if (nrComponents == 3)
        {
            dataFormat = GL_RGB;
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
        }
        else if (nrComponents == 4)
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW: error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW: error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
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
    }  std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: High"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: Medium"; break;
    case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: Low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}