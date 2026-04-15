#ifndef SHADERS_H
#define SHADERS_H

static const char* vertexShaderSource = R"(
#version 300 es
uniform mat4 u_MVPMatrix;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;

out vec4 v_Color;

void main() {
    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
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

#endif
