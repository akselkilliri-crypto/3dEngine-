#include "MatCapTexture.h"
#include <vector>
#include <cmath>

GLuint createProceduralMatCapTexture() {
    const int size = 256;
    std::vector<unsigned char> data(size * size * 4); // RGBA

    const unsigned char brightness = 200; // ~0.78 яркости

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float nx = (float)x / (size - 1) * 2.0f - 1.0f;
            float ny = (float)y / (size - 1) * 2.0f - 1.0f;
            float len2 = nx*nx + ny*ny;
            int idx = (y * size + x) * 4;

            if (len2 <= 1.0f) {
                // Внутри круга — однородный светло-серый
                data[idx]   = brightness;
                data[idx+1] = brightness;
                data[idx+2] = brightness;
                data[idx+3] = 255;
            } else {
                // Вне круга — чёрный, но эти области не используются
                data[idx] = data[idx+1] = data[idx+2] = 0;
                data[idx+3] = 255;
            }
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
