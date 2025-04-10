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

_This next part of the README is intended for the grading of my project submission in the course._

----

# Technical Report of Implementation

## 1. Project Overview

I built this point cloud renderer using C++ with OpenGL for drawing 3D points.
The project lets you load point cloud files (with `.pts` or `.ply` extensions) that include 3D positions, colors, and normal vectors.
Points are rendered as round sprites so the scene looks smooth and colorful.
I also added a simple light source to enhance the depth perception, 
and I used ImGui to create an on-screen menu where you can change things like point size, camera speed, and lighting options in real time.

---

## 2. How I Built It and What I Used

### 2.1. Application Structure

- **Main Loop and Setup:**  
  The main file (`main.cpp`) sets up the window using GLFW and loads OpenGL functions with GLAD.
  I also add callback functions for resizing the window, moving the mouse, and scrolling.
  This is where the main rendering loop lives, meaning it continuously draws the scene while processing user input.

- **Rendering the Scene:**  
  The scene is drawn using shaders.
  One shader is dedicated to drawing the points of the cloud, and another one is for drawing markers showing the light's position and direction.
  This separation makes it easier to manage and later improve the visuals.

- **Loading Data:**  
  The `PointRenderer` class (in `point_renderer.cpp` and `point_renderer.hpp`) is responsible for loading point cloud data from files.
  I built it to support both text-based (`.pts`) and binary (`.ply`) data files.

- **Camera and User Controls:**  
  The `Camera` class (in `camera.cpp` and `camera.hpp`) handles view transformations so you can move around the scene using keyboard and mouse controls.
  The `Menu` class (in `menu.cpp` and `menu.hpp`) uses ImGui to show a simple user interface, which makes adjusting settings really interactive.

### 2.2. Libraries

- **OpenGL:** Renders the graphics and handles drawing calls.
- **GLFW & GLAD:** GLFW creates the application window and captures input, while GLAD loads OpenGL functions.
- **GLM:** Provides math tools (like matrices and vectors) to help with camera movements and transformations.
- **ImGui:** Used for building the in-app menu that allows real-time adjustments.
- **C++17 Filesystem Library:** Manages file paths and checks if required files (like shaders or models) exist.
- **OpenCL (Optional):** Helps speed up the data loading for very large point clouds when enabled.
  - The OpenCL part was contributed with the help of ChatGPT (specifically the parallel loading function)
  - The main reason I tried it was to load the huge .pts file, but I was unable to test it because I could not get the OpenCL dependency to work.

---

## 3. Challenges in Visualizing Point Clouds

Working with point clouds is fun, but it comes with its own set of challenges:

- **Handling Large Data Sets:**  
  Point clouds can be huge. Rendering millions of points efficiently requires managing memory and making sure data is sent to the GPU quickly.
  - I had to optimize how I load and manage this data using OpenGL buffers.

- **Visual Clarity:**  
  If points are too small, they become hard to see; if they’re too large, they might overlap too much and hide details. Finding the right balance was key.

- **Lighting:**  
  Unlike traditional 3D models, point clouds don’t naturally form surfaces. This makes applying realistic lighting tricky because each point needs to be individually lit with its own normal vector, which might not always look perfect.
  - I personally feel that the lighting in my application looks quite bad and I don't really know why. 

---

## 4. Ideas for Improvements

- **Level of Detail (LOD):**  

  Implement techniques that lower the number of points rendered when the scene is very dense.
  This can boost performance and maintain smooth interactions.

- **Better Shading:**  

  Moving to more advanced lighting models like Phong shading or
  even physically based rendering (PBR) could make the scene look more realistic.

- **Interactive Features:**  

  Adding tools for filtering points or highlighting sections of the cloud would help in examining large datasets more effectively.

- **Use of Compute Shaders:**  

  Leveraging newer GPU features, such as compute shaders, might speed up data processing, making real-time updates even more efficient.

- **Lighting:**
  
  In the implementation, the lighting direction does not have an effect on the point cloud.
  I only implemented a lighting direction in the shader from the position of the light source and the fragment position.
  In other words, the light source is applied for each individual point, which renders it quite poorly.

---

## 5. Reflection

This point cloud renderer project allowed me to explore modern C++ techniques and real-time graphics programming.

Although the project already works well to visualize 3D point data with interactive controls,
there’s still plenty of room for improvement—in both performance and visual quality.
Working on this project helped me learn more about OpenGL, data management, and creating user interfaces using ImGui.

I hope to learn more about visual computing, since I believe I mostly only scratched surface and there's still much more that I need to fundamentally understand.