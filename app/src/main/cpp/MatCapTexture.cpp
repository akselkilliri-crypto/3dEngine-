#include "MatCapTexture.h"
#include <vector>
#include <cmath>

GLuint createProceduralMatCapTexture() {
    const int size = 256;
    std::vector<unsigned char> data(size * size * 4); // RGBA

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float nx = (float)x / (size - 1) * 2.0f - 1.0f;
            float ny = (float)y / (size - 1) * 2.0f - 1.0f;
            float nz = 0.0f;
            float len2 = nx*nx + ny*ny;
            if (len2 <= 1.0f) {
                nz = sqrtf(1.0f - len2);
            } else {
                int idx = (y * size + x) * 4;
                data[idx] = data[idx+1] = data[idx+2] = 0;
                data[idx+3] = 255;
                continue;
            }

            // Используем z-компоненту для имитации освещения сверху-спереди
            // Делаем яркость почти постоянной с небольшим затемнением по краям (когда |nx| или |ny| близки к 1)
            float edgeDarken = 1.0f - 0.15f * (fabs(nx) + fabs(ny)) / 2.0f; // от 0.85 до 1.0
            float brightness = 0.9f + 0.1f * nz; // от 0.8 до 1.0
            brightness *= edgeDarken;
            // Гарантируем, что яркость не меньше 0.8
            if (brightness < 0.8f) brightness = 0.8f;

            int idx = (y * size + x) * 4;
            unsigned char gray = (unsigned char)(brightness * 255);
            data[idx]   = gray;
            data[idx+1] = gray;
            data[idx+2] = gray;
            data[idx+3] = 255;
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}
