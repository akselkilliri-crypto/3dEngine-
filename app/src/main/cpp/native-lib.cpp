#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLUtils.h"
#include "Shaders.h"
#include "Mesh.h"
#include "MatCapTexture.h"

#define LOG_TAG "NativeLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static GLuint gProgram = 0;
static GLuint gMatCapTexture = 0;
static Mesh* gCubeMesh = nullptr;

static glm::mat4 gProjectionMatrix;
static glm::mat4 gViewMatrix;
static glm::mat4 gModelMatrix;

static int gWidth = 0, gHeight = 0;

// Инициализация куба с цветами вершин
void initCube() {
    // Half-Edge структура пока не используется, создаём простой массив вершин
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Куб от -1 до 1
    // Позиции и нормали (для простоты нормали вершин равны нормалям граней)
    // Цвета: каждая вершина имеет свой оттенок для наглядности
    // Грань передняя (z = 1)
    vertices.push_back({{-1, -1,  1}, { 0, 0, 1}, {1,0,0,1}}); // 0 красный
    vertices.push_back({{ 1, -1,  1}, { 0, 0, 1}, {0,1,0,1}}); // 1 зелёный
    vertices.push_back({{ 1,  1,  1}, { 0, 0, 1}, {0,0,1,1}}); // 2 синий
    vertices.push_back({{-1,  1,  1}, { 0, 0, 1}, {1,1,0,1}}); // 3 жёлтый

    // Грань задняя (z = -1)
    vertices.push_back({{-1, -1, -1}, { 0, 0,-1}, {1,0,1,1}}); // 4 пурпурный
    vertices.push_back({{ 1, -1, -1}, { 0, 0,-1}, {0,1,1,1}}); // 5 циан
    vertices.push_back({{ 1,  1, -1}, { 0, 0,-1}, {0.5,0.5,0.5,1}}); // 6 серый
    vertices.push_back({{-1,  1, -1}, { 0, 0,-1}, {1,0.5,0,1}}); // 7 оранжевый

    // Индексы для 12 треугольников
    // Передняя
    indices.insert(indices.end(), {0,1,2, 0,2,3});
    // Задняя
    indices.insert(indices.end(), {4,6,5, 4,7,6});
    // Левая
    indices.insert(indices.end(), {4,0,3, 4,3,7});
    // Правая
    indices.insert(indices.end(), {1,5,6, 1,6,2});
    // Верхняя
    indices.insert(indices.end(), {3,2,6, 3,6,7});
    // Нижняя
    indices.insert(indices.end(), {4,5,1, 4,1,0});

    gCubeMesh = new Mesh(vertices, indices);
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeInit(
        JNIEnv *env,
        jobject /* this */) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Компиляция шейдеров
    gProgram = createProgram(vertexShaderSource, fragmentShaderSource);
    if (gProgram == 0) {
        LOGE("Failed to create shader program");
        return;
    }

    // Создание процедурной MatCap текстуры
    gMatCapTexture = createProceduralMatCapTexture();

    // Инициализация геометрии куба
    initCube();

    // Настройка матриц (начальная камера)
    gModelMatrix = glm::mat4(1.0f);
    gViewMatrix = glm::lookAt(
        glm::vec3(0.0f, 2.0f, 5.0f),  // Позиция камеры
        glm::vec3(0.0f, 0.0f, 0.0f),  // Точка взгляда
        glm::vec3(0.0f, 1.0f, 0.0f)   // Вверх
    );
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeResize(
        JNIEnv *env,
        jobject /* this */,
        jint width,
        jint height) {
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
    float aspect = (float)width / (float)height;
    gProjectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeRender(
        JNIEnv *env,
        jobject /* this */) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gProgram == 0 || gCubeMesh == nullptr) return;

    glUseProgram(gProgram);

    // Передача матриц
    glm::mat4 mvp = gProjectionMatrix * gViewMatrix * gModelMatrix;
    GLint mvpLoc = glGetUniformLocation(gProgram, "u_MVPMatrix");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    glm::mat4 mv = gViewMatrix * gModelMatrix;
    GLint mvLoc = glGetUniformLocation(gProgram, "u_MVMatrix");
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));

    // Привязка MatCap текстуры
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gMatCapTexture);
    GLint texLoc = glGetUniformLocation(gProgram, "u_MatCap");
    glUniform1i(texLoc, 0);

    // Отрисовка куба
    gCubeMesh->draw(gProgram);
}

} // extern "C"
