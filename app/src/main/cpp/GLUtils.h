#ifndef GLUTILS_H
#define GLUTILS_H

#include <GLES3/gl3.h>

GLuint loadShader(GLenum type, const char* source);
GLuint createProgram(const char* vertexSource, const char* fragmentSource);

#endif
