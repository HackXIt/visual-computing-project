// Created by RINI on 03/04/2025.
// src/main.cpp
#include <filesystem>
#include <imgui.h>
#include "imgui_impl_glfw.h"        // Add this line
#include "imgui_impl_opengl3.h"     // And this line
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.hpp"
#include "point_renderer.hpp"
#include "camera.hpp"
#include "menu.hpp"

// settings
constexpr unsigned int SCR_WIDTH = 800;
constexpr unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Callback declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main(int argc, char* argv[])
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // GLFW initialization and configuration
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Point Cloud Renderer", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load all OpenGL function pointers with GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth test and point size program
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Check existence of shaders
    const std::filesystem::path vertexShaderPath = "shaders/point_cloud.vs";
    const std::filesystem::path fragmentShaderPath = "shaders/point_cloud.fs";
    if (!exists(vertexShaderPath) || !std::filesystem::exists(fragmentShaderPath))
    {
        std::cerr << "Shader files not found!" << std::endl;
        return -1;
    }

    // Create the shader for point cloud rendering
    Shader pointShader("shaders/point_cloud.vs", "shaders/point_cloud.fs");
    // Create the shader for marker rendering
    Shader markerShader("shaders/marker.vs", "shaders/marker.fs");

    std::string pointCloudFilePath = "resources/test.pts";
    // Check arguments if any
    if (argc > 1)
    {
        std::cout << "Arguments provided: ";
        for (int i = 1; i < argc; ++i)
        {
            std::cout << argv[i] << " ";
        }
        std::cout << std::endl;
        // Ignore all additional arguments, check first argument for valid file path
        std::filesystem::path filePath = argv[1];
        if (!std::filesystem::exists(filePath))
        {
            std::cerr << "File not found: " << filePath.string() << std::endl;
            return -1;
        }
        std::cout << "Using file: " << filePath.string() << std::endl;
        // Load the point cloud from the provided file
        // TODO: The PointRenderer class should be modified to accept a file path
        // and load the point cloud from that file.
        pointCloudFilePath = filePath.string();
    }

    // Setup ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // Set ImGui style.
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Menu menu;  // create an instance of the new Menu class

    PointRenderer renderer(pointCloudFilePath);

    // --- Alternative modes for comparison ---
    // Parallel OpenCL loading mode (for text-based .pts files)
    // if (renderer.loadPointCloudParallel(pointCloudFilePath))
    // {
    //     renderer.setupBuffers();
    // }

    // Binary mode loading (for .ply files)
    // if (renderer.loadPointCloudPly("resources/test.ply"))
    // {
    //     renderer.setupBuffers();
    // }

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltaTime for FPS calculations.
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input (this includes menu.processInput which also handles camera updates, etc.)
        menu.processInput(window, camera, deltaTime);

        // Start a new ImGui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the menu. (Pass the window pointer so the menu can lock/unlock the mouse.)
        menu.render(window, camera, renderer, deltaTime);

        // Clear the screen.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup transformation matrices.
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                static_cast<float>(SCR_WIDTH) / SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f); // Identity

        // --- Render the main point cloud ---
        pointShader.use();
        pointShader.setMat4("projection", projection);
        pointShader.setMat4("view", view);
        pointShader.setMat4("model", model);
        pointShader.setFloat("pointSize", menu.getPointSize());
        // Set lighting uniforms: pass the values from the menu.
        pointShader.setBool("useLighting", menu.getLightingEnabled());
        glm::vec3 offset = glm::vec3(0.0f, 0.0f, -1.0f); // or any small offset in view direction
        pointShader.setVec3("lightPos", menu.getLightingFollow() ? camera.Position + offset : menu.getLightPos());
        pointShader.setVec3("viewPos", camera.Position);
        pointShader.setVec3("lightColor", menu.getLightColor());
        renderer.render();

        // --- Render light markers using a marker shader ---
        markerShader.use();
        markerShader.setMat4("view", view);
        markerShader.setMat4("projection", projection);
        markerShader.setMat4("model", glm::mat4(1.0f)); // identity

        // 1. Render the light position marker (10.0 point size)
        glPointSize(10.0f);
        glm::vec3 lightMarker = menu.getLightingFollow() ? camera.Position : menu.getLightPos();
        markerShader.setVec3("markerColor", menu.getLightColor());
        {
            unsigned int markerVAO, markerVBO;
            glGenVertexArrays(1, &markerVAO);
            glGenBuffers(1, &markerVBO);
            glBindVertexArray(markerVAO);
            glBindBuffer(GL_ARRAY_BUFFER, markerVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &lightMarker, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
            glDrawArrays(GL_POINTS, 0, 1);
            glDeleteBuffers(1, &markerVBO);
            glDeleteVertexArrays(1, &markerVAO);
        }

        // 2. Render the light direction marker (1.0 point size)
        glm::vec3 lightDirNormalized = glm::normalize(menu.getLightDir());
        glm::vec3 dirMarker = menu.getLightPos() + lightDirNormalized * 1.0f;
        markerShader.setVec3("markerColor", menu.getLightColor());
        glPointSize(1.0f);
        {
            unsigned int markerVAO, markerVBO;
            glGenVertexArrays(1, &markerVAO);
            glGenBuffers(1, &markerVBO);
            glBindVertexArray(markerVAO);
            glBindBuffer(GL_ARRAY_BUFFER, markerVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &dirMarker, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
            glDrawArrays(GL_POINTS, 0, 1);
            glDeleteBuffers(1, &markerVBO);
            glDeleteVertexArrays(1, &markerVAO);
        }

        // Render the ImGui interface on top.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    // Only update the camera if the mouse is locked.
    if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
        return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}