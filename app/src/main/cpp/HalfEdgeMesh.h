#ifndef HALFEDGEMESH_H
#define HALFEDGEMESH_H

#include <vector>
#include <GLES3/gl3.h>
#include "Math.h"

struct VertexHE {
    Vec3 position;
    Vec3 normal;
    float color[4];
    int id;
};

struct Face {
    std::vector<int> vertexIndices;
    Vec3 normal;
    bool selected;
    Face() : selected(false) {}
};

class HalfEdgeMesh {
public:
    HalfEdgeMesh();
    ~HalfEdgeMesh();

    void createCube();
    void drawFaces(GLuint program, GLenum mode);
    void drawSelectedFaces(GLuint program, GLenum mode);
    void drawEdges(GLuint program, GLenum mode);

    int pickFace(const Mat4& mvp, const Mat4& view, int screenWidth, int screenHeight, float touchX, float touchY);
    void toggleFaceSelection(int faceIndex);

private:
    std::vector<VertexHE> vertices;
    std::vector<unsigned int> faceIndices;
    std::vector<unsigned int> selectedIndices;
    std::vector<Face> faces;

    GLuint VAO, VBO, EBO;
    bool dirty;

    void updateGPUBuffers();
    void updateSelectedBuffers();
};

#endif
