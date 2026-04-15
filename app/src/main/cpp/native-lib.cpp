#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>
#include <cmath>

#include "GLUtils.h"
#include "Shaders.h"
#include "Mesh.h"
#include "MatCapTexture.h"
#include "Math.h"

#define LOG_TAG "NativeLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static GLuint gProgram = 0;
static GLuint gMatCapTexture = 0;
static Mesh* gCubeMesh = nullptr;

static Mat4 gProjectionMatrix;
static Mat4 gViewMatrix;
static Mat4 gModelMatrix;

static int gWidth = 0, gHeight = 0;

// Переменные для вращения камеры
static float gYaw = 0.0f;      // Вокруг оси Y
static float gPitch = 0.0f;    // Вокруг оси X
static float gDistance = 5.0f; // Расстояние до центра
static Vec3 gTarget(0,0,0);

static float gLastX = 0, gLastY = 0;
static bool gDragging = false;

void initCube() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Куб от -1 до 1 с нормалями и цветами
    vertices.push_back({{-1,-1, 1}, {0,0,1}, {1,0,0,1}}); // 0
    vertices.push_back({{ 1,-1, 1}, {0,0,1}, {0,1,0,1}}); // 1
    vertices.push_back({{ 1, 1, 1}, {0,0,1}, {0,0,1,1}}); // 2
    vertices.push_back({{-1, 1, 1}, {0,0,1}, {1,1,0,1}}); // 3

    vertices.push_back({{-1,-1,-1}, {0,0,-1}, {1,0,1,1}}); // 4
    vertices.push_back({{ 1,-1,-1}, {0,0,-1}, {0,1,1,1}}); // 5
    vertices.push_back({{ 1, 1,-1}, {0,0,-1}, {0.5,0.5,0.5,1}}); // 6
    vertices.push_back({{-1, 1,-1}, {0,0,-1}, {1,0.5,0,1}}); // 7

    indices.insert(indices.end(), {0,1,2, 0,2,3}); // front
    indices.insert(indices.end(), {4,6,5, 4,7,6}); // back
    indices.insert(indices.end(), {4,0,3, 4,3,7}); // left
    indices.insert(indices.end(), {1,5,6, 1,6,2}); // right
    indices.insert(indices.end(), {3,2,6, 3,6,7}); // top
    indices.insert(indices.end(), {4,5,1, 4,1,0}); // bottom

    gCubeMesh = new Mesh(vertices, indices);
}

void updateViewMatrix() {
    // Преобразуем сферические координаты в декартовы
    float radYaw = gYaw * M_PI / 180.0f;
    float radPitch = gPitch * M_PI / 180.0f;
    Vec3 eye(
        gDistance * cosf(radPitch) * sinf(radYaw),
        gDistance * sinf(radPitch),
        gDistance * cosf(radPitch) * cosf(radYaw)
    );
    gViewMatrix = Mat4::lookAt(eye, gTarget, Vec3(0,1,0));
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeInit(JNIEnv*, jobject) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    gProgram = createProgram(vertexShaderSource, fragmentShaderSource);
    if (gProgram == 0) {
        LOGE("Failed to create shader program");
        return;
    }

    gMatCapTexture = createProceduralMatCapTexture();
    initCube();

    gModelMatrix = Mat4::identity();
    updateViewMatrix();
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeResize(JNIEnv*, jobject, jint w, jint h) {
    gWidth = w;
    gHeight = h;
    glViewport(0, 0, w, h);
    float aspect = (float)w / (float)h;
    gProjectionMatrix = Mat4::perspective(45.0f * M_PI / 180.0f, aspect, 0.1f, 100.0f);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeRender(JNIEnv*, jobject) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gProgram == 0 || gCubeMesh == nullptr) return;

    glUseProgram(gProgram);

    Mat4 mvp = gProjectionMatrix * gViewMatrix * gModelMatrix;
    GLint mvpLoc = glGetUniformLocation(gProgram, "u_MVPMatrix");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp.data);

    Mat4 mv = gViewMatrix * gModelMatrix;
    GLint mvLoc = glGetUniformLocation(gProgram, "u_MVMatrix");
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, mv.data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gMatCapTexture);
    GLint texLoc = glGetUniformLocation(gProgram, "u_MatCap");
    glUniform1i(texLoc, 0);

    gCubeMesh->draw(gProgram);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeOnTouchEvent(
        JNIEnv*, jobject, jint action, jfloat x, jfloat y) {
    switch (action) {
        case 0: // ACTION_DOWN
            gDragging = true;
            gLastX = x;
            gLastY = y;
            break;
        case 2: // ACTION_MOVE
            if (gDragging) {
                float dx = x - gLastX;
                float dy = y - gLastY;
                gYaw += dx * 0.5f;
                gPitch += dy * 0.5f;
                // Ограничим pitch, чтобы не переворачивать камеру
                if (gPitch > 89.0f) gPitch = 89.0f;
                if (gPitch < -89.0f) gPitch = -89.0f;
                updateViewMatrix();
                gLastX = x;
                gLastY = y;
            }
            break;
        case 1: // ACTION_UP
        case 3: // ACTION_CANCEL
            gDragging = false;
            break;
    }
}

} // extern "C"
