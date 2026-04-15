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

    // Теперь принимает отдельно матрицы проекции и вида
    int pickFace(const Mat4& proj, const Mat4& view, int screenWidth, int screenHeight, float touchX, float touchY);
    void toggleFaceSelection(int faceIndex);
    void subdivideSelected();

private:
    std::vector<VertexHE> vertices;
    std::vector<unsigned int> faceIndices;
    std::vector<unsigned int> selectedIndices;
    std::vector<unsigned int> edgeIndices;
    std::vector<Face> faces;

    GLuint VAO, VBO, EBO;
    bool dirty;
    bool edgesDirty;

    void updateGPUBuffers();
    void updateSelectedBuffers();
    void rebuildEdgeIndices();
    int addVertex(const VertexHE& v);

    bool rayIntersectsTriangle(const Vec3& rayOrigin, const Vec3& rayDir,
                               const Vec3& v0, const Vec3& v1, const Vec3& v2,
                               float& t);
};

#endif
