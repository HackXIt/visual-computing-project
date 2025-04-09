cmake_minimum_required(VERSION 3.14)
project(PointCloudRenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(glad STATIC
        external/glad/src/glad.c
)
target_include_directories(glad PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

include(FetchContent)

# Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

# Fetch GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# Fetch ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.91.9b
)
FetchContent_MakeAvailable(imgui)

# Build ImGui as a static library (include core and backends)
set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${glfw_SOURCE_DIR}/include
)

# Attempt to find OpenCL (necessary for parallel loading)
find_package(OpenCL)
if(OpenCL_FOUND)
    message(STATUS "OpenCL found, enabling parallel code.")
else()
    message(STATUS "OpenCL not found, disabling parallel code.")
endif()

# Add executable with source files
add_executable(PointCloudRenderer
        src/main.cpp
        src/camera.cpp
        src/shader.cpp
        src/point_renderer.cpp
        src/menu.cpp
)

target_include_directories(PointCloudRenderer PUBLIC
        ${glfw_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
        ${glm_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        $<$<BOOL:${OpenCL_FOUND}>:${OpenCL_INCLUDE_DIRS}>
)

# Link libraries
target_link_libraries(PointCloudRenderer
        glfw
        glad
        imgui
        $<$<BOOL:${OpenCL_FOUND}>:OpenCL::OpenCL>
)

# On Linux, link additional libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(PointCloudRenderer dl pthread)
endif()

# Define preprocessor symbols depending on OpenCL availability.
if(OpenCL_FOUND)
    target_compile_definitions(PointCloudRenderer PUBLIC HAVE_OPENCL)
else()
    target_compile_definitions(PointCloudRenderer PUBLIC NO_OPENCL)
endif()

# Copy the resources folder to the output directory after build
add_custom_command(TARGET PointCloudRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:PointCloudRenderer>/resources"
        COMMENT "Copying resources folder to output directory"
)
# Copy the shaders folder to the output directory
add_custom_command(TARGET PointCloudRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:PointCloudRenderer>/shaders"
        COMMENT "Copying shaders folder to output directory"
)