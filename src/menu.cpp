//
// Created by RINI on 09/04/2025.
//

#include "menu.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <filesystem>
#include <vector>
#include <cstdio>

extern bool firstMouse;

namespace fs = std::filesystem;

Menu::Menu()
    : pointSize(5.0f),
      lightColor(glm::vec3(1.0f, 1.0f, 1.0f)), // white light
      lightPos(glm::vec3(10.0f, 10.0f, 10.0f)), // light position in world space
      lightDir(glm::vec3(0.0f, -1.0f, 0.0f)), // light direction (default downward)
      lightingEnabled(true), // lighting enabled by default
      lightingFollow(true), // light follows camera by default
      openFileDialog(false),
      useFpsAverage(true), fpsHistoryMax(60) // average over last 60 frames
{
}

Menu::~Menu()
{
}

void Menu::render(GLFWwindow *window, Camera &camera, PointRenderer &renderer, float deltaTime)
{
    // Force menu window to appear at (10,10) with fixed size (300x500)
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(350, 470), ImGuiCond_Always);
    ImGui::Begin("Menu");

    // Lock mouse button: When clicked, fix the mouse cursor to the camera.
    if (ImGui::Button("Lock Mouse"))
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true; // Prevent large deltas from the previous unlocked position
    }

    // Help text.
    ImGui::Text("Controls:");
    ImGui::BulletText("W,A,S,D: Move camera");
    ImGui::BulletText("Q/E: Increase/Decrease point size");
    ImGui::BulletText("Up/Down: Increase/Decrease camera speed");
    ImGui::BulletText("Mouse: Look around (when locked)");
    ImGui::BulletText("SPACEBAR: Unlock mouse");
    ImGui::BulletText("Load File: Choose a new model");
    ImGui::BulletText("ESC: Exit");

    // FPS counter.
    float instantFps = 1.0f / (deltaTime > 0 ? deltaTime : 0.0001f);
    if (useFpsAverage)
    {
        fpsHistory.push_back(instantFps);
        if (fpsHistory.size() > static_cast<size_t>(fpsHistoryMax))
            fpsHistory.erase(fpsHistory.begin());
        float sum = 0.0f;
        for (float fps : fpsHistory)
            sum += fps;
        float avgFps = sum / fpsHistory.size();
        ImGui::Text("FPS AVG (%d frames): %.2f", static_cast<int>(fpsHistory.size()), avgFps);
    }
    else
    {
        ImGui::Text("FPS: %.2f", instantFps);
    }
    ImGui::Checkbox("Average FPS", &useFpsAverage);

    // Point size slider.
    ImGui::SliderFloat("Point Size", &pointSize, 1.0f, 100.0f, "%.0f");

    // Camera speed slider.
    ImGui::SliderFloat("Camera Speed", &camera.MovementSpeed, 1.0f, 25.0f, "%.1f");

    ImGui::Separator();
    ImGui::Text("Lighting Controls");

    if (lightingFollow)
    {
        // If the light follows the camera, set its position to the camera's position.
        lightPos = camera.Position;
    }

    // Light Position Input
    float lightPosArray[3] = { lightPos.x, lightPos.y, lightPos.z };
    if (ImGui::InputFloat3("Light Position", lightPosArray, "%.1f"))
    {
        lightPos = glm::vec3(lightPosArray[0], lightPosArray[1], lightPosArray[2]);
    }

    // Light Color Picker
    float lightColorArray[3] = { lightColor.r, lightColor.g, lightColor.b };
    if (ImGui::ColorEdit3("Light Color", lightColorArray))
    {
        lightColor = glm::vec3(lightColorArray[0], lightColorArray[1], lightColorArray[2]);
    }

    // Light direction input
    float lightDirArray[3] = { lightDir.x, lightDir.y, lightDir.z };
    if (ImGui::InputFloat3("Light Direction", lightDirArray))
        lightDir = glm::vec3(lightDirArray[0], lightDirArray[1], lightDirArray[2]);
    // Enable/Disable Lighting checkbox.
    ImGui::Checkbox("Enable Lighting", &lightingEnabled);
    // Follow camera checkbox.
    ImGui::Checkbox("Follow Camera", &lightingFollow);

    // Load file button.
    if (ImGui::Button("Load File"))
    {
        openFileDialog = true;
    }

    // Reset to defaults button.
    if (ImGui::Button("Reset"))
    {
        pointSize = 5.0f;
        camera.MovementSpeed = 2.5f;
        useFpsAverage = true;
    }

    // File chooser dialog.
    if (openFileDialog)
    {
        ImGui::Begin("File Chooser", &openFileDialog, ImGuiWindowFlags_AlwaysAutoResize);

        // Build the path for the resources directory (current directory + "/resources")
        fs::path resourceDir = fs::current_path() / "resources";

        if (fs::exists(resourceDir) && fs::is_directory(resourceDir))
        {
            ImGui::Text("Files in: %s", resourceDir.string().c_str());
            // List .pts and .ply files in the current directory.
            std::vector<std::string> files;
            for (const auto &entry : fs::directory_iterator(resourceDir))
            {
                if (entry.is_regular_file())
                {
                    auto ext = entry.path().extension().string();
                    if (ext == ".pts" || ext == ".ply")
                    {
                        files.push_back(entry.path().string());
                    }
                }
            }

            // Display files for selection.
            for (const auto &f : files)
            {
                if (ImGui::Selectable(f.c_str()))
                {
                    selectedFile = f;
                    openFileDialog = false;
                    if (!renderer.loadPointCloud(selectedFile))
                    {
                        printf("Failed to load file: %s\n", selectedFile.c_str());
                    }
                }
            }
        }
        else
        {
            ImGui::Text("Resources folder not found: %s", resourceDir.string().c_str());
        }
        ImGui::End();
    }
    ImGui::End();
}

void Menu::processInput(GLFWwindow *window, Camera &camera, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // use spacebar to unlock the mouse if needed.
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (pointSize < 100.0f)
        {
            pointSize += 1.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (pointSize > 1.0f)
        {
            pointSize -= 1.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        if (camera.MovementSpeed < 25.0f)
        {
            camera.MovementSpeed += 0.1f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        if (camera.MovementSpeed > 1.0f)
        {
            camera.MovementSpeed -= 0.1f;
        }
    }
}
