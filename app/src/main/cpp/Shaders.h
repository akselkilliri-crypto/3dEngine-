#ifndef SHADERS_H
#define SHADERS_H

// Шейдер для граней (MatCap)
static const char* vertexShaderSource = R"(
#version 300 es
uniform mat4 u_MVPMatrix;
uniform mat4 u_MVMatrix;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;

out vec3 v_Normal;
out vec4 v_Color;

void main() {
    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
    v_Normal = mat3(u_MVMatrix) * a_Normal;
    v_Color = a_Color;
}
)";

static const char* fragmentShaderSource = R"(
#version 300 es
precision highp float;

in vec3 v_Normal;
in vec4 v_Color;

uniform sampler2D u_MatCap;

out vec4 fragColor;

void main() {
    vec3 normal = normalize(v_Normal);
    vec2 matcapCoord = normal.xy * 0.5 + 0.5;
    vec4 matcapColor = texture(u_MatCap, matcapCoord);
    
    // Добавляем ambient составляющую, чтобы куб никогда не был полностью чёрным
    float ambient = 0.3;
    vec4 finalColor = (ambient + matcapColor) * v_Color;
    fragColor = finalColor;
}
)";

// Шейдер для рёбер (простой, цвет uniform)
static const char* edgeVertexShaderSource = R"(
#version 300 es
uniform mat4 u_MVPMatrix;
layout(location = 0) in vec3 a_Position;

void main() {
    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
}
)";

static const char* edgeFragmentShaderSource = R"(
#version 300 es
precision highp float;
uniform vec4 u_Color;
out vec4 fragColor;

void main() {
    fragColor = u_Color;
}
)";

#endif
