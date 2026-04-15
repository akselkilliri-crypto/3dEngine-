#include <jni.h>
#include <GLES3/gl3.h>

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeInit(
        JNIEnv *env,
        jobject /* this */) {
    // Устанавливаем серый цвет очистки
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeResize(
        JNIEnv *env,
        jobject /* this */,
        jint width,
        jint height) {
    glViewport(0, 0, width, height);
}

JNIEXPORT void JNICALL
Java_com_example_modelinengine_MyGLRenderer_nativeRender(
        JNIEnv *env,
        jobject /* this */) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // extern "C"
