# visual-computing-project
A project for the course visual computing of my masters programme.

# Point Cloud Renderer

This project implements a point cloud renderer using OpenGL and GLFW.

It reads a `.pts` or `.ply` file containing 3D positions, RGB colors, and normal vectors, and renders the points as round point sprites.

- The point size can be adjusted in real time using the Q/E keys.
- Camera controls (W/A/S/D and mouse for view) allow navigation through the scene.
- Additionally, the camera speed can be adjusted with the UP/DOWN arrow keys.

## Project Structure

```plaintext
/
├── CMakeLists.txt
├── README.md
├── external/
│   └── glad/
│       ├── include/
│       │   ├── glad/glad.h
│       │   └── KHR/khrplatform.h
│       └── src/
│           └── glad.c
├── resources/
│   └── ... various point cloud files to load...
├── shaders/
│   ├── point_cloud.vs
│   └── point_cloud.fs
└── src/
    ├── main.cpp
    ├── camera.cpp
    ├── camera.hpp
    ├── menu.cpp
    ├── menu.hpp
    ├── point_renderer.cpp
    ├── point_renderer.hpp
    ├── shader.cpp
    └── shader.hpp
```


## Build Instructions

This project uses CMake and FetchContent to automatically download external dependencies (GLFW, GLAD, GLM, ImGui).

### Requirements

**NOTE:** I'm a student and these requirements are based on my own system. They do not reflect the *possible* minimum requirements for this project, only what I ran them with.

- CMake 3.14 or newer
- A C++17 compliant compiler
- Preferably windows. Linux should work too, but I have not tested it.
  - *There are no specific Windows APIs being used.*
  - *I tried to make sure the `CMakeLists.txt` allows for cross-platform compilation.*

### Steps
1. Configure the project:
   ```bash
   cmake -B build .
   ```
2. Build the project:
   ```bash
    cmake --build build --config Release --parallel
    ```
3. Run the executable:
   ```bash
   # Linux
   cd build/Release
   ./PointCloudRenderer
   # Windows
   cd .\build\Release
   .\PointCloudRenderer.exe
   ```
   
## Controls

- **W/A/S/D:** Move the camera 
- **Mouse:** Look around 
- **SPACEBAR:** Unlock mouse cursor to interact with menu
- **Q/E:** Increase/decrease point size
- **UP/DOWN Arrow:** Increase/decrease camera speed 
- **ESC:** Exit the application

## Menu

The application features a simple ImGui-based menu that allows you to:
- Help text for controls.
- Load a point cloud file.
- Adjust the point size and camera speed via sliders.
- Reset variables (point size, camera speed) to their default values.

## Discussion

The project renders a point cloud by:

- Loading a custom `.pts` or `.ply` file format.
- Setting up a VAO/VBO for the point data.
- Using a shader program to transform vertices and apply per-point colors.
- Creating round point sprites in the fragment shader using gl_PointCoord.