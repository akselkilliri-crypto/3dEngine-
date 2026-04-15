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

            // Нормализованный вектор (nx, ny, nz) используется для генерации освещения
            // Создаём более равномерное освещение с акцентом на переднюю полусферу
            float front = fmax(0.0f, nz);                    // 0..1, 1 когда смотрит прямо на камеру
            float sideX = fabs(nx);
            float sideY = fabs(ny);
            
            // Базовая яркость от 0.5 до 1.0, зависит от ориентации
            float brightness = 0.6f + 0.4f * front + 0.2f * sideX + 0.2f * sideY;
            if (brightness > 1.0f) brightness = 1.0f;
            if (brightness < 0.4f) brightness = 0.4f;  // никогда не будет совсем тёмным

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
