#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>
#include <cmath>
#include <algorithm>

#include "GLUtils.h"
#include "Shaders.h"
#include "MatCapTexture.h"
#include "Math.h"
#include "HalfEdgeMesh.h"

#define LOG_TAG "NativeLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static GLuint gProgram = 0;
static GLuint gEdgeProgram = 0;
static GLuint gHighlightProgram = 0;
static GLuint gMatCapTexture = 0;

static HalfEdgeMesh* gMesh = nullptr;

static Mat4 gProjectionMatrix;
static Mat4 gViewMatrix;
static Mat4 gModelMatrix;

static int gWidth = 0, gHeight = 0;

// Параметры камеры
static float gYaw = 0.0f;
static float gPitch = 0.0f;
static float gDistance = 5.0f;
static Vec3 gTarget(0,0,0);

static float gLastX = 0, gLastY = 0;
static bool gDragging = false;

void updateViewMatrix() {
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
    gEdgeProgram = createProgram(edgeVertexShaderSource, edgeFragmentShaderSource);
    gHighlightProgram = createProgram(highlightVertexShaderSource, highlightFragmentShaderSource);
    if (gProgram == 0 || gEdgeProgram == 0 || gHighlightProgram == 0) {
        LOGE("Failed to create shader programs");
        return;
    }

    gMatCapTexture = createProceduralMatCapTexture();

    gMesh = new HalfEdgeMesh();
    gMesh->createCube();

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

    if (gProgram == 0 || gMesh == nullptr) return;

    Mat4 mvp = gProjectionMatrix * gViewMatrix * gModelMatrix;
    Mat4 mv = gViewMatrix * gModelMatrix;

    // === Грани ===
    glUseProgram(gProgram);
    GLint mvpLoc = glGetUniformLocation(gProgram, "u_MVPMatrix");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp.data);
    GLint mvLoc = glGetUniformLocation(gProgram, "u_MVMatrix");
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, mv.data);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gMatCapTexture);
    GLint texLoc = glGetUniformLocation(gProgram, "u_MatCap");
    glUniform1i(texLoc, 0);
    gMesh->drawFaces(gProgram, GL_TRIANGLES);

    // === Выделенные грани ===
    glUseProgram(gHighlightProgram);
    mvpLoc = glGetUniformLocation(gHighlightProgram, "u_MVPMatrix");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp.data);
    GLint colorLoc = glGetUniformLocation(gHighlightProgram, "u_Color");
    glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    gMesh->drawSelectedFaces(gHighlightProgram, GL_TRIANGLES);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // === Рёбра ===
    glUseProgram(gEdgeProgram);
    mvpLoc = glGetUniformLocation(gEdgeProgram, "u_MVPMatrix");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp.data);
    colorLoc = glGetUniformLocation(gEdgeProgram, "u_Color");
    glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    gMesh->drawEdges(gEdgeProgram, GL_LINES);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_CULL_FACE);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeOnTouchEvent(
        JNIEnv*, jobject, jint action, jfloat x, jfloat y) {
    static bool tapHandled = false;
    switch (action) {
        case 0: // ACTION_DOWN
            gDragging = true;
            gLastX = x;
            gLastY = y;
            tapHandled = false;
            break;
        case 2: // ACTION_MOVE
            if (gDragging) {
                float dx = x - gLastX;
                float dy = y - gLastY;
                if (fabs(dx) > 10 || fabs(dy) > 10) tapHandled = true;
                gYaw -= dx * 0.5f;
                gPitch += dy * 0.5f;
                if (gPitch > 89.0f) gPitch = 89.0f;
                if (gPitch < -89.0f) gPitch = -89.0f;
                updateViewMatrix();
                gLastX = x;
                gLastY = y;
            }
            break;
        case 1: // ACTION_UP
            if (!tapHandled) {
                int faceIdx = gMesh->pickFace(gProjectionMatrix, gViewMatrix, gWidth, gHeight, x, y);
                if (faceIdx >= 0) {
                    gMesh->toggleFaceSelection(faceIdx);
                }
            }
            gDragging = false;
            break;
        case 3: // ACTION_CANCEL
            gDragging = false;
            break;
    }
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeOnScale(
        JNIEnv*, jobject, jfloat scaleFactor) {
    gDistance /= scaleFactor;
    if (gDistance < 2.0f) gDistance = 2.0f;
    if (gDistance > 100.0f) gDistance = 100.0f; // было 20
    updateViewMatrix();
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeSubdivide(JNIEnv*, jobject) {
    if (gMesh) {
        gMesh->subdivideSelected();
    }
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeExtrude(JNIEnv*, jobject, jfloat distance) {
    if (gMesh) {
        gMesh->extrudeSelected(distance);
    }
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativePan(
        JNIEnv*, jobject, jfloat dx, jfloat dy, jint screenWidth, jint screenHeight) {
    // Перемещаем gTarget в плоскости, перпендикулярной направлению взгляда
    // Определяем векторы "вправо" и "вверх" в мировых координатах
    float radYaw = gYaw * M_PI / 180.0f;
    float radPitch = gPitch * M_PI / 180.0f;
    Vec3 forward(
        cosf(radPitch) * sinf(radYaw),
        sinf(radPitch),
        cosf(radPitch) * cosf(radYaw)
    );
    Vec3 up(0,1,0);
    Vec3 right = forward.cross(up).normalized();
    Vec3 camUp = right.cross(forward).normalized();

    // Масштабируем смещение в зависимости от расстояния
    float scale = gDistance * 0.005f; // эмпирический коэффициент
    gTarget = gTarget + right * (dx * scale) + camUp * (-dy * scale);
    updateViewMatrix();
}

} // extern "C"
