#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <vector>

#define LOG_TAG "NativeLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct Vertex {
    float pos[3];
    float color[4];
};

static GLuint gProgram = 0;
static GLuint gVBO = 0, gEBO = 0, gVAO = 0;
static int gIndexCount = 0;

// Простейший вершинный шейдер без матриц
static const char* vertexShaderSource = R"(
#version 300 es
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
out vec4 v_Color;
void main() {
    gl_Position = vec4(a_Position, 1.0);
    v_Color = a_Color;
}
)";

static const char* fragmentShaderSource = R"(
#version 300 es
precision highp float;
in vec4 v_Color;
out vec4 fragColor;
void main() {
    fragColor = v_Color;
}
)";

GLuint loadShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            char* log = new char[len];
            glGetShaderInfoLog(shader, len, nullptr, log);
            LOGE("Shader compile error: %s", log);
            delete[] log;
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint createProgram(const char* vs, const char* fs) {
    GLuint v = loadShader(GL_VERTEX_SHADER, vs);
    GLuint f = loadShader(GL_FRAGMENT_SHADER, fs);
    if (!v || !f) return 0;
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    GLint linked;
    glGetProgramiv(p, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint len;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            char* log = new char[len];
            glGetProgramInfoLog(p, len, nullptr, log);
            LOGE("Program link error: %s", log);
            delete[] log;
        }
        glDeleteProgram(p);
        p = 0;
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

void initCube() {
    // Вершины куба в NDC (от -1 до 1) с яркими цветами
    Vertex vertices[] = {
        // Передняя грань (z=1)
        {{-0.5f, -0.5f,  0.5f}, {1,0,0,1}},
        {{ 0.5f, -0.5f,  0.5f}, {0,1,0,1}},
        {{ 0.5f,  0.5f,  0.5f}, {0,0,1,1}},
        {{-0.5f,  0.5f,  0.5f}, {1,1,0,1}},
        // Задняя грань (z=-0.5)
        {{-0.5f, -0.5f, -0.5f}, {1,0,1,1}},
        {{ 0.5f, -0.5f, -0.5f}, {0,1,1,1}},
        {{ 0.5f,  0.5f, -0.5f}, {1,1,1,1}},
        {{-0.5f,  0.5f, -0.5f}, {1,0.5,0,1}}
    };
    unsigned int indices[] = {
        0,1,2, 0,2,3, // front
        4,6,5, 4,7,6, // back
        4,0,3, 4,3,7, // left
        1,5,6, 1,6,2, // right
        3,2,6, 3,6,7, // top
        4,5,1, 4,1,0  // bottom
    };
    gIndexCount = 36;

    glGenVertexArrays(1, &gVAO);
    glGenBuffers(1, &gVBO);
    glGenBuffers(1, &gEBO);

    glBindVertexArray(gVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    LOGI("Cube initialized: VBO=%u, EBO=%u, VAO=%u", gVBO, gEBO, gVAO);
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeInit(JNIEnv*, jobject) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    gProgram = createProgram(vertexShaderSource, fragmentShaderSource);
    if (!gProgram) {
        LOGE("Failed to create program");
        return;
    }
    LOGI("Program created: %u", gProgram);

    initCube();

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) LOGE("OpenGL error after init: 0x%x", err);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeResize(JNIEnv*, jobject, jint w, jint h) {
    glViewport(0, 0, w, h);
    LOGI("Resize: %dx%d", w, h);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeRender(JNIEnv*, jobject) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!gProgram) return;

    glUseProgram(gProgram);
    glBindVertexArray(gVAO);
    glDrawElements(GL_TRIANGLES, gIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) LOGE("Render error: 0x%x", err);
}

}
