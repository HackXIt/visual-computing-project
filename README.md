# visual-computing-project
A project for the course visual computing of my masters programme.

# Point Cloud Renderer

This project implements a point cloud renderer using OpenGL and GLFW.

It reads a `.pts` or `.ply` file containing 3D positions, RGB colors, and normal vectors, and renders the points as round point sprites.

- The point size can be adjusted in real time using the Q/E keys.
- Camera controls (W/A/S/D and mouse for view) allow navigation through the scene.
- Additionally, the camera speed can be adjusted with the UP/DOWN arrow keys.

The application also features a simplistic light source, which can be moved around with the camera.
- The renderer uses shaders to apply per-point colors and lighting effects.
- The light source can be toggled on/off and its position, color, and direction can be adjusted through the menu.

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
│   ├── marker.fs
│   ├── marker.vs
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
  - *All dependencies should be cross-platform compatible.*

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
- Adjust light-source position, color and direction
- Enable/Disable lighting
- Enable/Disable light-source following camera
- Reset variables (point size, camera speed) to their default values.

## Point-Cloud Files

The repository does not include any point cloud files.
The only file included is the `test.pts` with 3 points colored red, green, and blue.

The `test.pts` is loaded by default when the program starts with no command line arguments.

To load your own point cloud file, you can either:
- Place it under the `resources/` folder and load it through the menu.
- Supply the file name as a command line argument when running the program.
  
  *(e.g., `./PointCloudRenderer resources/test.pts`)* 

When loading through the menu, the program lists all `.pts` and `.ply` files in the `resources/` directory of the build directory.

You can place your own point cloud files in the `resources/` directory of the repository, the `cmake` build will automatically copy them to the build output directory.

The expected format for the `.pts` file is:
```plaintext
// Comments before the number of points are ignored, afterwards they cause errors
<Number of points>
<Points in format: X Y Z Rf Gf Bf Nx Ny Nz>
```
- `X`, `Y`, `Z`: 3D coordinates of the point.
- `Rf`, `Gf`, `Bf`: RGB color values (0-255).
- `Nx`, `Ny`, `Nz`: Normal vector components.j
- The first line of the file can contain comments, which are ignored until the first number.
- The first number indicates the number of points in the file.
- The rest of the lines contain the point data.
- For an example, see the [`test.pts`](resources/test.pts) file in the [`resources/`](resources/) directory.

The expected format for the `.ply` file is:
```plaintext
ply
format binary_little_endian 1.0
element vertex <number of vertices>  
property float x
property float y
property float z
property float nx
property float ny
property float nz
property uchar red
property uchar green
property uchar blue
property uchar class
end_header
<binary data>
```
- `x`, `y`, `z`: 3D coordinates of the point.
- `nx`, `ny`, `nz`: Normal vector components.
- `red`, `green`, `blue`: RGB color values (0-255).
- `class`: Class label (not used in this project).
- Binary data follows the header, containing the point data in the specified order.
- The number of vertices is specified in the header.
- The binary data is read in chunks corresponding to the number of vertices.
- For more information, see [PLY file format](https://en.wikipedia.org/wiki/PLY_(file_format)).

----

_This part of the README is intended for the grading part of my project submission in the course._

----

# Technical Analysis of Implementation

The project renders a point cloud by:

- Loading a custom `.pts` or `.ply` file format.
- Setting up a VAO/VBO for the point data.
- Using a shader program to transform vertices and apply per-point colors.
- Creating round point sprites in the fragment shader using gl_PointCoord.

