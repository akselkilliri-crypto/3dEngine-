#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>
#include <cmath>

#include "GLUtils.h"
#include "Shaders.h"
#include "Mesh.h"
#include "Math.h"

#define LOG_TAG "NativeLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static GLuint gProgram = 0;
static Mesh* gCubeMesh = nullptr;

static Mat4 gProjectionMatrix;
static Mat4 gViewMatrix;
static Mat4 gModelMatrix;

static int gWidth = 0, gHeight = 0;

void initCube() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Яркие цвета для граней
    vertices.push_back({{-1,-1, 1}, {0,0,1}, {1,0,0,1}}); // 0 красный
    vertices.push_back({{ 1,-1, 1}, {0,0,1}, {0,1,0,1}}); // 1 зелёный
    vertices.push_back({{ 1, 1, 1}, {0,0,1}, {0,0,1,1}}); // 2 синий
    vertices.push_back({{-1, 1, 1}, {0,0,1}, {1,1,0,1}}); // 3 жёлтый

    vertices.push_back({{-1,-1,-1}, {0,0,-1}, {1,0,1,1}}); // 4 пурпурный
    vertices.push_back({{ 1,-1,-1}, {0,0,-1}, {0,1,1,1}}); // 5 циан
    vertices.push_back({{ 1, 1,-1}, {0,0,-1}, {1,1,1,1}}); // 6 белый
    vertices.push_back({{-1, 1,-1}, {0,0,-1}, {1,0.5,0,1}}); // 7 оранжевый

    indices.insert(indices.end(), {0,1,2, 0,2,3}); // front
    indices.insert(indices.end(), {4,6,5, 4,7,6}); // back
    indices.insert(indices.end(), {4,0,3, 4,3,7}); // left
    indices.insert(indices.end(), {1,5,6, 1,6,2}); // right
    indices.insert(indices.end(), {3,2,6, 3,6,7}); // top
    indices.insert(indices.end(), {4,5,1, 4,1,0}); // bottom

    gCubeMesh = new Mesh(vertices, indices);
    LOGI("Cube mesh created with %zu vertices, %zu indices", vertices.size(), indices.size());
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
    LOGI("Shader program created successfully");

    initCube();

    gModelMatrix = Mat4::identity();
    gViewMatrix = Mat4::lookAt(Vec3(0,2,5), Vec3(0,0,0), Vec3(0,1,0));
    LOGI("View matrix set");
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeResize(JNIEnv*, jobject, jint width, jint height) {
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
    float aspect = (float)width / (float)height;
    gProjectionMatrix = Mat4::perspective(45.0f * M_PI / 180.0f, aspect, 0.1f, 100.0f);
    LOGI("Resize: %dx%d, aspect=%f", width, height, aspect);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeRender(JNIEnv*, jobject) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gProgram == 0 || gCubeMesh == nullptr) {
        LOGE("Render: program or mesh is null");
        return;
    }

    glUseProgram(gProgram);

    Mat4 mvp = gProjectionMatrix * gViewMatrix * gModelMatrix;
    GLint mvpLoc = glGetUniformLocation(gProgram, "u_MVPMatrix");
    if (mvpLoc == -1) {
        LOGE("Uniform u_MVPMatrix not found");
    }
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp.data);

    gCubeMesh->draw(gProgram);

    // Проверка ошибок OpenGL
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("OpenGL error: 0x%x", err);
    }
}

}
