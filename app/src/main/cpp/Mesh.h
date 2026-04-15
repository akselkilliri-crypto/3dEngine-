#ifndef MESH_H
#define MESH_H

#include <GLES3/gl3.h>
#include <vector>
#include "Math.h"

struct Vertex {
    Vec3 position;
    Vec3 normal;
    float color[4];
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();
    void draw(GLuint program, GLenum mode);

private:
    GLuint VAO, VBO, EBO;
    size_t indexCount;
};

#endif
