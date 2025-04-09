//
// Created by RINI on 03/04/2025.
//

#ifndef POINT_RENDERER_HPP
#define POINT_RENDERER_HPP

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Point {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
};

class PointRenderer {
public:
    explicit PointRenderer(const std::string& filename, bool parallel = false);
    ~PointRenderer();

    // Render the point cloud
    void render() const;

    // Load the point cloud using the stored filename.
    bool loadPointCloud();
    // Clear the existing data and load from a new file (updates the member filename).
    bool loadPointCloud(const std::string& filename);

private:
    // These methods load based on file format
    bool loadPointCloudPts(const std::string& filename);
    bool loadPointCloudPly(const std::string& filename);
    bool loadPointCloudParallel(const std::string& filename);

    // Once points are loaded, this method (re)creates the OpenGL buffers.
    void setupBuffers();

    std::vector<Point> points;
    unsigned int VAO, VBO;
    std::string filename;
    bool parallelLoading;
};


#endif // POINT_RENDERER_HPP
