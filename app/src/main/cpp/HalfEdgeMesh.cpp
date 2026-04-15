#include "HalfEdgeMesh.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>

#define LOG_TAG "HalfEdgeMesh"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

HalfEdgeMesh::HalfEdgeMesh() : VAO(0), VBO(0), EBO(0), dirty(true) {}

HalfEdgeMesh::~HalfEdgeMesh() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
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
    updateGPUBuffers();
    glBindVertexArray(VAO);
    static const unsigned int edgeIndices[] = {
        0,1, 1,2, 2,3, 3,0,
        4,5, 5,6, 6,7, 7,4,
        0,4, 1,5, 2,6, 3,7
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(edgeIndices), edgeIndices, GL_STATIC_DRAW);
    glDrawElements(mode, 24, GL_UNSIGNED_INT, 0);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices.size() * sizeof(unsigned int), faceIndices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

int HalfEdgeMesh::pickFace(const Mat4& mvp, const Mat4& view, int screenWidth, int screenHeight, float touchX, float touchY) {
    float ndcX = (2.0f * touchX) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * touchY) / screenHeight;

    Mat4 invMVP = Mat4::inverse(mvp);

    Vec4 nearPointNDC(ndcX, ndcY, -1.0f, 1.0f);
    Vec4 nearPointWorld = invMVP * nearPointNDC;
    nearPointWorld = Vec4(nearPointWorld.x / nearPointWorld.w, nearPointWorld.y / nearPointWorld.w, nearPointWorld.z / nearPointWorld.w, 1.0f);

    Vec4 farPointNDC(ndcX, ndcY, 1.0f, 1.0f);
    Vec4 farPointWorld = invMVP * farPointNDC;
    farPointWorld = Vec4(farPointWorld.x / farPointWorld.w, farPointWorld.y / farPointWorld.w, farPointWorld.z / farPointWorld.w, 1.0f);

    Vec3 rayOrigin(nearPointWorld.x, nearPointWorld.y, nearPointWorld.z);
    Vec3 rayDir(farPointWorld.x - nearPointWorld.x, farPointWorld.y - nearPointWorld.y, farPointWorld.z - nearPointWorld.z);
    rayDir = rayDir.normalized();

    float tmin = -1e9f, tmax = 1e9f;
    float t1, t2;
    if (fabs(rayDir.x) > 1e-6f) {
        t1 = (-1.0f - rayOrigin.x) / rayDir.x;
        t2 = (1.0f - rayOrigin.x) / rayDir.x;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return -1;
    } else if (rayOrigin.x < -1.0f || rayOrigin.x > 1.0f) return -1;

    if (fabs(rayDir.y) > 1e-6f) {
        t1 = (-1.0f - rayOrigin.y) / rayDir.y;
        t2 = (1.0f - rayOrigin.y) / rayDir.y;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return -1;
    } else if (rayOrigin.y < -1.0f || rayOrigin.y > 1.0f) return -1;

    if (fabs(rayDir.z) > 1e-6f) {
        t1 = (-1.0f - rayOrigin.z) / rayDir.z;
        t2 = (1.0f - rayOrigin.z) / rayDir.z;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return -1;
    } else if (rayOrigin.z < -1.0f || rayOrigin.z > 1.0f) return -1;

    if (tmin < 0) return -1;

    Vec3 hit = rayOrigin + rayDir * tmin;
    float dx = fabs(hit.x) - 1.0f;
    float dy = fabs(hit.y) - 1.0f;
    float dz = fabs(hit.z) - 1.0f;
    if (fabs(dx) < 0.01f) {
        return (hit.x > 0) ? 3 : 2;
    } else if (fabs(dy) < 0.01f) {
        return (hit.y > 0) ? 4 : 5;
    } else {
        return (hit.z > 0) ? 0 : 1;
    }
}

void HalfEdgeMesh::toggleFaceSelection(int faceIndex) {
    if (faceIndex >= 0 && faceIndex < (int)faces.size()) {
        faces[faceIndex].selected = !faces[faceIndex].selected;
        updateSelectedBuffers();
    }
}
