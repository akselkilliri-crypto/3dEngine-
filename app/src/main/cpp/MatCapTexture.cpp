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

            // Используем нормаль для создания освещения, но гарантируем яркость от 0.7 до 1.0
            float front = fmax(0.0f, nz);
            float sideX = fabs(nx);
            float sideY = fabs(ny);
            
            float brightness = 0.7f + 0.3f * front + 0.1f * sideX + 0.1f * sideY;
            if (brightness > 1.0f) brightness = 1.0f;
            // Минимальная яркость теперь 0.7, так что тёмных областей не будет

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
