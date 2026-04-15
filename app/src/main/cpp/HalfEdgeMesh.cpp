#include "HalfEdgeMesh.h"
#include <android/log.h>
#include <cmath>
#include <algorithm>

#define LOG_TAG "HalfEdgeMesh"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// ... (конструктор, деструктор, createCube, updateGPUBuffers, updateSelectedBuffers, drawFaces, drawSelectedFaces) без изменений ...

int HalfEdgeMesh::pickFace(const Mat4& mvp, const Mat4& view, int screenWidth, int screenHeight, float touchX, float touchY) {
    // Преобразуем координаты касания в нормализованные координаты устройства (NDC)
    float ndcX = (2.0f * touchX) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * touchY) / screenHeight;

    // Обратная матрица MVP
    Mat4 invMVP = Mat4::inverse(mvp);

    // Точка на ближней плоскости (z = -1 в NDC)
    Vec4 nearPointNDC(ndcX, ndcY, -1.0f, 1.0f);
    Vec4 nearPointWorld = invMVP * nearPointNDC;
    nearPointWorld = nearPointWorld * (1.0f / nearPointWorld.w);

    // Точка на дальней плоскости (z = 1 в NDC)
    Vec4 farPointNDC(ndcX, ndcY, 1.0f, 1.0f);
    Vec4 farPointWorld = invMVP * farPointNDC;
    farPointWorld = farPointWorld * (1.0f / farPointWorld.w);

    // Направление луча в мировых координатах
    Vec3 rayOrigin(nearPointWorld.x, nearPointWorld.y, nearPointWorld.z);
    Vec3 rayDir(farPointWorld.x - nearPointWorld.x,
                farPointWorld.y - nearPointWorld.y,
                farPointWorld.z - nearPointWorld.z);
    rayDir = rayDir.normalized();

    // Куб от -1 до 1 по всем осям
    float tmin = -1e9f, tmax = 1e9f;
    // Пересечение с AABB куба
    float t1, t2;
    // X
    if (fabs(rayDir.x) > 1e-6f) {
        t1 = (-1.0f - rayOrigin.x) / rayDir.x;
        t2 = (1.0f - rayOrigin.x) / rayDir.x;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return -1;
    } else if (rayOrigin.x < -1.0f || rayOrigin.x > 1.0f) return -1;

    // Y
    if (fabs(rayDir.y) > 1e-6f) {
        t1 = (-1.0f - rayOrigin.y) / rayDir.y;
        t2 = (1.0f - rayOrigin.y) / rayDir.y;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return -1;
    } else if (rayOrigin.y < -1.0f || rayOrigin.y > 1.0f) return -1;

    // Z
    if (fabs(rayDir.z) > 1e-6f) {
        t1 = (-1.0f - rayOrigin.z) / rayDir.z;
        t2 = (1.0f - rayOrigin.z) / rayDir.z;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return -1;
    } else if (rayOrigin.z < -1.0f || rayOrigin.z > 1.0f) return -1;

    if (tmin < 0) return -1; // луч начинается внутри куба? Игнорируем

    // Точка пересечения
    Vec3 hit = rayOrigin + rayDir * tmin;
    // Определяем грань по наибольшей компоненте нормали (с учётом знака)
    float dx = fabs(hit.x) - 1.0f;
    float dy = fabs(hit.y) - 1.0f;
    float dz = fabs(hit.z) - 1.0f;
    // Ищем компоненту, которая ближе всего к границе
    if (fabs(dx) < 0.01f) {
        return (hit.x > 0) ? 3 : 2; // правая или левая
    } else if (fabs(dy) < 0.01f) {
        return (hit.y > 0) ? 4 : 5; // верхняя или нижняя
    } else {
        return (hit.z > 0) ? 0 : 1; // передняя или задняя
    }
}
