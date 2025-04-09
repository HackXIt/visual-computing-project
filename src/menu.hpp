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
    // 'window' is used for unlocking the mouse.
    // 'renderer' is used to load a new file if selected.
    // 'deltaTime' is the time elapsed since the last frame for FPS calculation.
    void Menu::render(GLFWwindow *window, Camera &camera, PointRenderer &renderer, float deltaTime);

    // Returns the current point size, as set in the slider.
    float getPointSize() const { return pointSize; }
    void setPointSize(float size) { pointSize = size; }
    void Menu::processInput(GLFWwindow *window, Camera &camera, float deltaTime);

private:
    float pointSize;           // Current point size (1 to 100)
    bool openFileDialog;       // Whether the file chooser dialog is open
    std::string selectedFile;  // Stores the selected file path

    // FPS averaging members.
    bool useFpsAverage;               // If true, display averaged FPS (default: true)
    std::vector<float> fpsHistory;    // History of FPS measurements
    int fpsHistoryMax;                // Number of frames over which to average FPS
};

#endif // MENU_HPP

