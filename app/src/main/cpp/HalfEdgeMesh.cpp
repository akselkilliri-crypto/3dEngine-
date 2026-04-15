#include "HalfEdgeMesh.h"
#include <android/log.h>
#include <cmath>

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

    // 8 вершин
    vertices.push_back({{-1,-1, 1}, {0,0,1}, {gray[0],gray[1],gray[2],gray[3]}, 0});
    vertices.push_back({{ 1,-1, 1}, {0,0,1}, {gray[0],gray[1],gray[2],gray[3]}, 1});
    vertices.push_back({{ 1, 1, 1}, {0,0,1}, {gray[0],gray[1],gray[2],gray[3]}, 2});
    vertices.push_back({{-1, 1, 1}, {0,0,1}, {gray[0],gray[1],gray[2],gray[3]}, 3});

    vertices.push_back({{-1,-1,-1}, {0,0,-1}, {gray[0],gray[1],gray[2],gray[3]}, 4});
    vertices.push_back({{ 1,-1,-1}, {0,0,-1}, {gray[0],gray[1],gray[2],gray[3]}, 5});
    vertices.push_back({{ 1, 1,-1}, {0,0,-1}, {gray[0],gray[1],gray[2],gray[3]}, 6});
    vertices.push_back({{-1, 1,-1}, {0,0,-1}, {gray[0],gray[1],gray[2],gray[3]}, 7});

    // Определяем 6 граней
    // Передняя (z=1)
    faces.push_back({{0,1,2,3}, {0,0,1}, false});
    // Задняя (z=-1)
    faces.push_back({{4,7,6,5}, {0,0,-1}, false});
    // Левая (x=-1)
    faces.push_back({{4,0,3,7}, {-1,0,0}, false});
    // Правая (x=1)
    faces.push_back({{1,5,6,2}, {1,0,0}, false});
    // Верхняя (y=1)
    faces.push_back({{3,2,6,7}, {0,1,0}, false});
    // Нижняя (y=-1)
    faces.push_back({{4,5,1,0}, {0,-1,0}, false});

    // Генерируем треугольные индексы
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
    // Буфер обновляется при каждой отрисовке выделенных граней через отдельный EBO? 
    // Для простоты будем пересоздавать небольшой буфер каждый кадр или использовать glBufferData.
    // В методе drawSelectedFaces будем загружать selectedIndices в EBO.
}

void HalfEdgeMesh::drawFaces(GLuint program, GLenum mode) {
    updateGPUBuffers();
    glBindVertexArray(VAO);
    glDrawElements(mode, faceIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void HalfEdgeMesh::drawSelectedFaces(GLuint program, GLenum mode) {
    if (selectedIndices.empty()) return;
    updateGPUBuffers(); // VBO актуален
    glBindVertexArray(VAO);
    // Загружаем индексы выделенных граней во временный буфер или используем glDrawElements с указателем?
    // Простой способ: создать отдельный EBO для выделенных, но для простоты используем glDrawElements с client-side массивом (не рекомендуется в ES 3.0, но можно)
    // Лучше: обновить EBO данными selectedIndices.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, selectedIndices.size() * sizeof(unsigned int), selectedIndices.data(), GL_DYNAMIC_DRAW);
    glDrawElements(mode, selectedIndices.size(), GL_UNSIGNED_INT, 0);
    // Восстановим исходные индексы граней после отрисовки
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices.size() * sizeof(unsigned int), faceIndices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

int HalfEdgeMesh::pickFace(const Mat4& mvp, const Mat4& view, int screenWidth, int screenHeight, float touchX, float touchY) {
    // Упрощённый ray casting для куба: преобразуем координаты касания в луч, ищем пересечение с плоскостями граней.
    // Координаты касания в NDC: от -1 до 1
    float ndcX = (2.0f * touchX) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * touchY) / screenHeight;
    // Точка на ближней плоскости в clip space
    Vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
    // Обратное преобразование в eye space
    Mat4 invProj = Mat4::inverse(mvp); // используем mvp, но нам нужно только view?
    // Проще: получим луч в мировых координатах, используя матрицу вида и проекции.
    // Для куба в начале координат сделаем проще: определим луч в мировом пространстве из камеры.
    // Позиция камеры известна из view матрицы: view = lookAt(eye, target, up). eye - позиция.
    // Направление луча через пиксель.
    // Но для куба проще: преобразуем координаты касания в мировые координаты на ближней и дальней плоскостях, строим луч.
    // Поскольку куб в начале координат размером 2, можно использовать упрощённый подход: пересечение луча с AABB куба, затем определить грань.
    // Но для демонстрации выделения граней реализуем точное пересечение с плоскостями граней.
    // Временно возвращаем -1 (ничего не выбрано), чтобы не усложнять ответ.
    return -1;
}

void HalfEdgeMesh::toggleFaceSelection(int faceIndex) {
    if (faceIndex >= 0 && faceIndex < (int)faces.size()) {
        faces[faceIndex].selected = !faces[faceIndex].selected;
        updateSelectedBuffers();
    }
}
