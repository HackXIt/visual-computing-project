//
// Created by RINI on 09/04/2025.
//

#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <vector>
#include "point_renderer.hpp"
#include <GLFW/glfw3.h> // Needed for GLFWwindow*
#include <camera.hpp>

// The Menu class uses ImGui to display an interactive UI overlay.
// It provides a point size slider, a load file button with a file chooser,
// an unlock mouse button, help text, and an FPS counter with optional averaging.
class Menu {
public:
    Menu();
    ~Menu();

    // Render the menu.
    // 'window' is used for locking/unlocking the mouse.
    // 'camera' is used for adjusting camera properties.
    // 'renderer' is used for model reloading.
    // 'deltaTime' is time elapsed since last frame.
    void Menu::render(GLFWwindow *window, Camera &camera, PointRenderer &renderer, float deltaTime);

    // Returns the current point size, as set in the slider.
    float getPointSize() const { return pointSize; }
    void setPointSize(float size) { pointSize = size; }

    // Light property accessors
    glm::vec3 getLightColor() const { return lightColor; }
    void setLightColor(glm::vec3 color) { lightColor = color; }
    glm::vec3 getLightPos() const { return lightPos; }
    void setLightPos(glm::vec3 pos) { lightPos = pos; }
    void Menu::processInput(GLFWwindow *window, Camera &camera, float deltaTime);
    glm::vec3 getLightDir() const { return lightDir; }
    void setLightDir(const glm::vec3 &dir) { lightDir = dir; }
    bool getLightingEnabled() const { return lightingEnabled; }
    bool getLightingFollow() const { return lightingFollow; }

private:
    float pointSize;           // Current point size (1 to 100)
    glm::vec3 lightColor;      // Light color (RGB) for the light source
    glm::vec3 lightPos;        // Light position (XYZ) in world space
    glm::vec3 lightDir;        // Light direction (XYZ) for the marker
    bool lightingEnabled;      // Toggle for lighting (true = enabled, default)
    bool lightingFollow;       // Toggle for light position following the camera
    bool openFileDialog;       // Whether the file chooser dialog is open
    std::string selectedFile;  // Stores the selected file path

    // FPS averaging members.
    bool useFpsAverage;               // If true, display averaged FPS (default: true)
    std::vector<float> fpsHistory;    // History of FPS measurements
    int fpsHistoryMax;                // Number of frames over which to average FPS
};

#endif // MENU_HPP

