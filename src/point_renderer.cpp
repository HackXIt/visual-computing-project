//
// Created by RINI on 03/04/2025.
//

#include "point_renderer.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <filesystem>  // C++17 filesystem header
#ifdef HAVE_OPENCL
  #include <CL/cl.h>
#endif

namespace fs = std::filesystem;

PointRenderer::PointRenderer(const std::string &file, bool parallel)
    : VAO(0), VBO(0), filename(file), parallelLoading(parallel)
{
    if (!loadPointCloud())
        std::cerr << "Failed to load point cloud from file: " << file << std::endl;
}

PointRenderer::~PointRenderer() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}


void PointRenderer::render() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));
    glBindVertexArray(0);
}

void PointRenderer::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), points.data(), GL_STATIC_DRAW);

    // position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, position));

    // color attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)offsetof(Point, color));

    glBindVertexArray(0);
}

bool PointRenderer::loadPointCloud()
{
    fs::path filePath(filename);
    if (!fs::exists(filePath)) {
        std::cerr << "File does not exist: " << filename << std::endl;
        return false;
    }

    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    bool loadOk = false;
    points.clear();

    if (ext == ".pts") {
        if (parallelLoading)
            loadOk = loadPointCloudParallel(filename);
        else
            loadOk = loadPointCloudPts(filename);
    } else if (ext == ".ply") {
        loadOk = loadPointCloudPly(filename);
    } else {
        std::cerr << "Unsupported file extension: " << ext << std::endl;
        return false;
    }

    if (loadOk)
        setupBuffers();

    return loadOk;
}

bool PointRenderer::loadPointCloud(const std::string &newFilename)
{
    // Update the member filename.
    filename = newFilename;
    points.clear();

    return loadPointCloud();
}

bool PointRenderer::loadPointCloudPts(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()){
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    int numPoints = 0;
    std::string line;
    // Read the first non-comment line to get the point count.
    while (std::getline(file, line)) {
        if (line.size() >= 2 && line[0]=='/' && line[1]=='/')
            continue;
        std::istringstream iss(line);
        if (!(iss >> numPoints)) {
            std::cerr << "Failed to parse number of points from line: " << line << std::endl;
            return false;
        }
        break;
    }
    points.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        Point pt;
        file >> pt.position.x >> pt.position.y >> pt.position.z
             >> pt.color.r >> pt.color.g >> pt.color.b
             >> pt.normal.x >> pt.normal.y >> pt.normal.z;
        if (file.fail()) {
            std::cerr << "Failed to read point " << i << std::endl;
            return false;
        }
        points.push_back(pt);
    }
    std::cout << "Loaded " << points.size() << " points from PTS file." << std::endl;
    return true;
}

#ifdef HAVE_OPENCL
bool PointRenderer::loadPointCloudParallel(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()){
        std::cerr << "[Parallel Mode] Failed to open file: " << filename << std::endl;
        return false;
    }
    int numPoints = 0;
    std::string line;
    // Read first non-comment line for number of points.
    while (std::getline(file, line)) {
        if (line.size() >= 2 && line[0]=='/' && line[1]=='/')
            continue;
        std::istringstream iss(line);
        if (!(iss >> numPoints)) {
            std::cerr << "[Parallel Mode] Failed to parse number of points from line: " << line << std::endl;
            return false;
        }
        break;
    }
    // Read all remaining float values.
    std::vector<float> floatData;
    floatData.reserve(numPoints * 9);
    float value;
    while (file >> value)
        floatData.push_back(value);
    file.close();

    if (floatData.size() != static_cast<size_t>(numPoints * 9)) {
        std::cerr << "[Parallel Mode] Expected " << (numPoints * 9)
                  << " values, but got " << floatData.size() << std::endl;
        return false;
    }

    cl_int clStatus;
    cl_uint numPlatforms;
    clStatus = clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (clStatus != CL_SUCCESS || numPlatforms == 0) {
        std::cerr << "[Parallel Mode] No OpenCL platforms found." << std::endl;
        return false;
    }
    std::vector<cl_platform_id> platforms(numPlatforms);
    clStatus = clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
    cl_platform_id platform = platforms[0];

    cl_uint numDevices;
    clStatus = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 0, nullptr, &numDevices);
    if (clStatus != CL_SUCCESS || numDevices == 0) {
        std::cerr << "[Parallel Mode] No OpenCL devices found." << std::endl;
        return false;
    }
    std::vector<cl_device_id> devices(numDevices);
    clStatus = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, numDevices, devices.data(), nullptr);
    cl_device_id device = devices[0];

    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &clStatus);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to create OpenCL context." << std::endl;
        return false;
    }
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &clStatus);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to create OpenCL command queue." << std::endl;
        clReleaseContext(context);
        return false;
    }

    size_t dataSize = floatData.size() * sizeof(float);
    cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, dataSize, floatData.data(), &clStatus);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to create input buffer." << std::endl;
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return false;
    }
    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, dataSize, nullptr, &clStatus);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to create output buffer." << std::endl;
        clReleaseMemObject(inputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return false;
    }

    // Define a simple kernel that copies data.
    const char* kernelSource = R"CLC(
        __kernel void copy_data(__global const float* input, __global float* output) {
            int i = get_global_id(0);
            output[i] = input[i];
        }
    )CLC";
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, &clStatus);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to create program." << std::endl;
        clReleaseMemObject(outputBuffer);
        clReleaseMemObject(inputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return false;
    }
    clStatus = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (clStatus != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> buildLog(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, buildLog.data(), nullptr);
        std::cerr << "[Parallel Mode] Error in kernel: " << buildLog.data() << std::endl;
        clReleaseProgram(program);
        clReleaseMemObject(outputBuffer);
        clReleaseMemObject(inputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return false;
    }
    cl_kernel kernel = clCreateKernel(program, "copy_data", &clStatus);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to create kernel." << std::endl;
        clReleaseProgram(program);
        clReleaseMemObject(outputBuffer);
        clReleaseMemObject(inputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return false;
    }
    clStatus  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer);
    clStatus |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuffer);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to set kernel arguments." << std::endl;
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseMemObject(outputBuffer);
        clReleaseMemObject(inputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return false;
    }
    size_t globalSize = floatData.size();
    clStatus = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to enqueue kernel." << std::endl;
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseMemObject(outputBuffer);
        clReleaseMemObject(inputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return false;
    }
    clFinish(queue);
    std::vector<float> outputData(floatData.size());
    clStatus = clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, dataSize, outputData.data(), 0, nullptr, nullptr);
    if (clStatus != CL_SUCCESS) {
        std::cerr << "[Parallel Mode] Failed to read output buffer." << std::endl;
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseMemObject(outputBuffer);
        clReleaseMemObject(inputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return false;
    }
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(outputBuffer);
    clReleaseMemObject(inputBuffer);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    points.resize(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        int base = i * 9;
        points[i].position = glm::vec3(outputData[base], outputData[base + 1], outputData[base + 2]);
        points[i].color    = glm::vec3(outputData[base + 3], outputData[base + 4], outputData[base + 5]);
        points[i].normal   = glm::vec3(outputData[base + 6], outputData[base + 7], outputData[base + 8]);
    }
    std::cout << "[Parallel Mode] Loaded " << points.size() << " points." << std::endl;
    return true;
}
#else
// If OpenCL is not available, provide a stub implementation.
bool PointRenderer::loadPointCloudParallel(const std::string &filename)
{
    std::cerr << "Parallel loading disabled: OpenCL not found." << std::endl;
    return false;
}
#endif // HAVE_OPENCL

bool PointRenderer::loadPointCloudPly(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()){
        std::cerr << "[PLY Mode] Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int numPoints = 0;
    bool headerEnded = false;
    // Parse header: look for "element vertex" and "end_header"
    while (std::getline(file, line)) {
        if (line.find("element vertex") != std::string::npos) {
            std::istringstream iss(line);
            std::string token;
            // Expected: "element vertex <numPoints>"
            iss >> token; // "element"
            iss >> token; // "vertex"
            iss >> numPoints;
        }
        if (line == "end_header") {
            headerEnded = true;
            break;
        }
    }

    if (!headerEnded) {
        std::cerr << "[PLY Mode] 'end_header' not found in file." << std::endl;
        return false;
    }

    std::cout << "[PLY Mode] Number of points in PLY: " << numPoints << std::endl;

    // Define a packed structure to match the binary layout in the file.
    #pragma pack(push, 1)
    struct PlyVertex {
        float x, y, z;
        float nx, ny, nz;
        unsigned char r, g, b, cls;
    };
    #pragma pack(pop)

    size_t vertexSize = sizeof(PlyVertex); // expected to be 28 bytes
    std::vector<PlyVertex> vertices(numPoints);

    file.read(reinterpret_cast<char*>(vertices.data()), numPoints * vertexSize);
    if (!file) {
        std::cerr << "[PLY Mode] Error reading binary PLY data." << std::endl;
        return false;
    }
    file.close();

    points.resize(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        points[i].position = glm::vec3(vertices[i].x, vertices[i].y, vertices[i].z);
        points[i].normal   = glm::vec3(vertices[i].nx, vertices[i].ny, vertices[i].nz);
        // Convert color from 0-255 to [0,1] range.
        points[i].color    = glm::vec3(vertices[i].r / 255.0f, vertices[i].g / 255.0f, vertices[i].b / 255.0f);
    }

    std::cout << "[PLY Mode] Loaded " << points.size() << " points." << std::endl;
    return true;
}

