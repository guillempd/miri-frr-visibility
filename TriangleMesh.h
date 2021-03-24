#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE

#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"

// Class TriangleMesh renders a very simple room with textures

class TriangleMesh
{

public:
    TriangleMesh();

    void addVertex(const glm::vec3 &position);
    void addTriangle(int v0, int v1, int v2);

    void buildCube();

    void sendToOpenGL(ShaderProgram &program);
    void render() const;
    void free();

private:
    std::vector<glm::vec3> vertices;
    std::vector<int> triangles;

    GLuint vao;
    GLuint vbo;
    GLint posLocation, normalLocation;
};

#endif // _TRIANGLE_MESH_INCLUDE
