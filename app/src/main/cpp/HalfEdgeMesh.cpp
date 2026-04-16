#include "HalfEdgeMesh.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>
#include <set>

#define LOG_TAG "HalfEdgeMesh"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

HalfEdgeMesh::HalfEdgeMesh() : VAO(0), VBO(0), EBO(0), dirty(true), edgesDirty(true) {}

HalfEdgeMesh::~HalfEdgeMesh() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

int HalfEdgeMesh::addVertex(const VertexHE& v) {
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (vertices[i].position.x == v.position.x &&
            vertices[i].position.y == v.position.y &&
            vertices[i].position.z == v.position.z) {
            return i;
        }
    }
    vertices.push_back(v);
    return vertices.size() - 1;
}

void HalfEdgeMesh::rebuildEdgeIndices() {
    edgeIndices.clear();
    std::set<std::pair<int,int>> edges;
    for (const auto& face : faces) {
        int n = face.vertexIndices.size();
        for (int i = 0; i < n; ++i) {
            int a = face.vertexIndices[i];
            int b = face.vertexIndices[(i+1)%n];
            if (a > b) std::swap(a,b);
            edges.insert({a,b});
        }
    }
    for (const auto& e : edges) {
        edgeIndices.push_back(e.first);
        edgeIndices.push_back(e.second);
    }
    edgesDirty = false;
}

void HalfEdgeMesh::createCube() {
    vertices.clear();
    faces.clear();
    faceIndices.clear();

    float gray[4] = {0.7f, 0.7f, 0.7f, 1.0f};

    vertices.push_back({{-1,-1, 1}, {0,0,1}, {gray[0],gray[1],gray[2],gray[3]}, 0});
    vertices.push_back({{ 1,-1, 1}, {0,0,1}, {gray[0],gray[1],gray[2],gray[3]}, 1});
    vertices.push_back({{ 1, 1, 1}, {0,0,1}, {gray[0],gray[1],gray[2],gray[3]}, 2});
    vertices.push_back({{-1, 1, 1}, {0,0,1}, {gray[0],gray[1],gray[2],gray[3]}, 3});

    vertices.push_back({{-1,-1,-1}, {0,0,-1}, {gray[0],gray[1],gray[2],gray[3]}, 4});
    vertices.push_back({{ 1,-1,-1}, {0,0,-1}, {gray[0],gray[1],gray[2],gray[3]}, 5});
    vertices.push_back({{ 1, 1,-1}, {0,0,-1}, {gray[0],gray[1],gray[2],gray[3]}, 6});
    vertices.push_back({{-1, 1,-1}, {0,0,-1}, {gray[0],gray[1],gray[2],gray[3]}, 7});

    Face f;
    f.vertexIndices = {0,1,2,3}; f.normal = {0,0,1}; f.selected = false;
    faces.push_back(f);
    f.vertexIndices = {4,7,6,5}; f.normal = {0,0,-1}; f.selected = false;
    faces.push_back(f);
    f.vertexIndices = {4,0,3,7}; f.normal = {-1,0,0}; f.selected = false;
    faces.push_back(f);
    f.vertexIndices = {1,5,6,2}; f.normal = {1,0,0}; f.selected = false;
    faces.push_back(f);
    f.vertexIndices = {3,2,6,7}; f.normal = {0,1,0}; f.selected = false;
    faces.push_back(f);
    f.vertexIndices = {4,5,1,0}; f.normal = {0,-1,0}; f.selected = false;
    faces.push_back(f);

    for (size_t i = 0; i < faces.size(); ++i) {
        const auto& face = faces[i];
        if (face.vertexIndices.size() == 4) {
            faceIndices.push_back(face.vertexIndices[0]);
            faceIndices.push_back(face.vertexIndices[1]);
            faceIndices.push_back(face.vertexIndices[2]);
            faceIndices.push_back(face.vertexIndices[0]);
            faceIndices.push_back(face.vertexIndices[2]);
            faceIndices.push_back(face.vertexIndices[3]);
        }
    }

    dirty = true;
    edgesDirty = true;
    LOGI("Cube created with %zu vertices, %zu faces", vertices.size(), faces.size());
}

void HalfEdgeMesh::updateGPUBuffers() {
    if (!dirty) return;

    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexHE), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices.size() * sizeof(unsigned int), faceIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexHE), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexHE), (void*)offsetof(VertexHE, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexHE), (void*)offsetof(VertexHE, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    dirty = false;
}

void HalfEdgeMesh::updateSelectedBuffers() {
    selectedIndices.clear();
    for (size_t i = 0; i < faces.size(); ++i) {
        if (faces[i].selected) {
            const auto& f = faces[i];
            if (f.vertexIndices.size() == 4) {
                selectedIndices.push_back(f.vertexIndices[0]);
                selectedIndices.push_back(f.vertexIndices[1]);
                selectedIndices.push_back(f.vertexIndices[2]);
                selectedIndices.push_back(f.vertexIndices[0]);
                selectedIndices.push_back(f.vertexIndices[2]);
                selectedIndices.push_back(f.vertexIndices[3]);
            }
        }
    }
}

void HalfEdgeMesh::drawFaces(GLuint program, GLenum mode) {
    updateGPUBuffers();
    glBindVertexArray(VAO);
    glDrawElements(mode, faceIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void HalfEdgeMesh::drawSelectedFaces(GLuint program, GLenum mode) {
    if (selectedIndices.empty()) return;
    updateGPUBuffers();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, selectedIndices.size() * sizeof(unsigned int), selectedIndices.data(), GL_DYNAMIC_DRAW);
    glDrawElements(mode, selectedIndices.size(), GL_UNSIGNED_INT, 0);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices.size() * sizeof(unsigned int), faceIndices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void HalfEdgeMesh::drawEdges(GLuint program, GLenum mode) {
    if (edgesDirty) rebuildEdgeIndices();
    if (edgeIndices.empty()) return;
    updateGPUBuffers();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(unsigned int), edgeIndices.data(), GL_STATIC_DRAW);
    glDrawElements(mode, edgeIndices.size(), GL_UNSIGNED_INT, 0);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices.size() * sizeof(unsigned int), faceIndices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

bool HalfEdgeMesh::rayIntersectsTriangle(const Vec3& rayOrigin, const Vec3& rayDir,
                                         const Vec3& v0, const Vec3& v1, const Vec3& v2,
                                         float& t) {
    const float EPSILON = 1e-6f;
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;
    Vec3 h = rayDir.cross(edge2);
    float a = edge1.dot(h);
    if (fabs(a) < EPSILON) return false;
    float f = 1.0f / a;
    Vec3 s = rayOrigin - v0;
    float u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f) return false;
    Vec3 q = s.cross(edge1);
    float v = f * rayDir.dot(q);
    if (v < 0.0f || u + v > 1.0f) return false;
    t = f * edge2.dot(q);
    return (t > EPSILON);
}

// Функция unproject для преобразования экранных координат в мировые
static Vec3 unProject(float winX, float winY, float winZ,
                      const Mat4& proj, const Mat4& view,
                      int viewport[4]) {
    Mat4 inv = Mat4::inverse(proj * view);
    float ndcX = (winX - viewport[0]) / viewport[2] * 2.0f - 1.0f;
    float ndcY = (winY - viewport[1]) / viewport[3] * 2.0f - 1.0f;
    float ndcZ = winZ * 2.0f - 1.0f;
    Vec4 in(ndcX, ndcY, ndcZ, 1.0f);
    Vec4 out = inv * in;
    if (out.w == 0.0f) return Vec3(0,0,0);
    return Vec3(out.x / out.w, out.y / out.w, out.z / out.w);
}

int HalfEdgeMesh::pickFace(const Mat4& proj, const Mat4& view, int screenWidth, int screenHeight, float touchX, float touchY) {
    // viewport: (0,0,screenWidth,screenHeight)
    int viewport[4] = {0, 0, screenWidth, screenHeight};

    // Инвертируем Y, так как в OpenGL (0,0) внизу слева, а касание сверху
    float glY = screenHeight - touchY;

    Vec3 nearWorld = unProject(touchX, glY, 0.0f, proj, view, viewport);
    Vec3 farWorld  = unProject(touchX, glY, 1.0f, proj, view, viewport);

    Vec3 rayOrigin = nearWorld;
    Vec3 rayDir = (farWorld - nearWorld).normalized();

    int closestFace = -1;
    float minT = 1e9f;

    // Перебираем все грани и их треугольники
    for (size_t i = 0; i < faces.size(); ++i) {
        const Face& face = faces[i];
        if (face.vertexIndices.size() < 3) continue;

        // Триангуляция (для квадрата два треугольника)
        for (size_t j = 1; j < face.vertexIndices.size() - 1; ++j) {
            int idx0 = face.vertexIndices[0];
            int idx1 = face.vertexIndices[j];
            int idx2 = face.vertexIndices[j+1];
            Vec3 v0 = vertices[idx0].position;
            Vec3 v1 = vertices[idx1].position;
            Vec3 v2 = vertices[idx2].position;
            float t;
            if (rayIntersectsTriangle(rayOrigin, rayDir, v0, v1, v2, t)) {
                if (t < minT) {
                    minT = t;
                    closestFace = i;
                }
            }
        }
    }
    return closestFace;
}

void HalfEdgeMesh::toggleFaceSelection(int faceIndex) {
    if (faceIndex >= 0 && faceIndex < (int)faces.size()) {
        faces[faceIndex].selected = !faces[faceIndex].selected;
        updateSelectedBuffers();
    }
}

void HalfEdgeMesh::subdivideSelected() {
    std::vector<Face> newFaces;
    std::vector<Face> facesToRemove;

    for (size_t i = 0; i < faces.size(); ++i) {
        if (faces[i].selected) {
            facesToRemove.push_back(faces[i]);
        } else {
            newFaces.push_back(faces[i]);
        }
    }

    if (facesToRemove.empty()) return;

    for (const Face& face : facesToRemove) {
        if (face.vertexIndices.size() != 4) continue;

        int v0 = face.vertexIndices[0];
        int v1 = face.vertexIndices[1];
        int v2 = face.vertexIndices[2];
        int v3 = face.vertexIndices[3];

        Vec3 p0 = vertices[v0].position;
        Vec3 p1 = vertices[v1].position;
        Vec3 p2 = vertices[v2].position;
        Vec3 p3 = vertices[v3].position;

        Vec3 p01 = (p0 + p1) * 0.5f;
        Vec3 p12 = (p1 + p2) * 0.5f;
        Vec3 p23 = (p2 + p3) * 0.5f;
        Vec3 p30 = (p3 + p0) * 0.5f;
        Vec3 center = (p0 + p1 + p2 + p3) * 0.25f;

        Vec3 normal = face.normal;
        float gray[4] = {0.7f, 0.7f, 0.7f, 1.0f};

        VertexHE v01, v12, v23, v30, vc;
        v01.position = p01; v01.normal = normal; for(int c=0;c<4;++c) v01.color[c] = gray[c];
        v12.position = p12; v12.normal = normal; for(int c=0;c<4;++c) v12.color[c] = gray[c];
        v23.position = p23; v23.normal = normal; for(int c=0;c<4;++c) v23.color[c] = gray[c];
        v30.position = p30; v30.normal = normal; for(int c=0;c<4;++c) v30.color[c] = gray[c];
        vc.position = center; vc.normal = normal; for(int c=0;c<4;++c) vc.color[c] = gray[c];

        int idx01 = addVertex(v01);
        int idx12 = addVertex(v12);
        int idx23 = addVertex(v23);
        int idx30 = addVertex(v30);
        int idxC = addVertex(vc);

        Face f1, f2, f3, f4;
        f1.vertexIndices = {v0, idx01, idxC, idx30};
        f2.vertexIndices = {idx01, v1, idx12, idxC};
        f3.vertexIndices = {idxC, idx12, v2, idx23};
        f4.vertexIndices = {idx30, idxC, idx23, v3};

        f1.normal = f2.normal = f3.normal = f4.normal = normal;
        f1.selected = f2.selected = f3.selected = f4.selected = false;

        newFaces.push_back(f1);
        newFaces.push_back(f2);
        newFaces.push_back(f3);
        newFaces.push_back(f4);
    }

    faces = newFaces;

    faceIndices.clear();
    for (size_t i = 0; i < faces.size(); ++i) {
        const auto& f = faces[i];
        if (f.vertexIndices.size() == 4) {
            faceIndices.push_back(f.vertexIndices[0]);
            faceIndices.push_back(f.vertexIndices[1]);
            faceIndices.push_back(f.vertexIndices[2]);
            faceIndices.push_back(f.vertexIndices[0]);
            faceIndices.push_back(f.vertexIndices[2]);
            faceIndices.push_back(f.vertexIndices[3]);
        }
    }

    dirty = true;
    edgesDirty = true;
    updateSelectedBuffers();
    LOGI("Subdivision complete. Total faces: %zu, vertices: %zu", faces.size(), vertices.size());
}
