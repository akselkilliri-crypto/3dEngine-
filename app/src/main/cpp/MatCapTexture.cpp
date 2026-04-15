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

            float r = fmax(0.0f, nx) * 0.8f + fmax(0.0f, ny) * 0.2f + 0.2f;
            float g = fmax(0.0f, ny) * 0.7f + fmax(0.0f, -nx) * 0.3f + 0.1f;
            float b = fmax(0.0f, nz) * 0.9f + fmax(0.0f, -ny) * 0.3f + 0.1f;

            float fresnel = powf(1.0f - fabs(nz), 2.0f);
            r += fresnel * 0.5f;
            g += fresnel * 0.3f;
            b += fresnel * 0.2f;

            int idx = (y * size + x) * 4;
            data[idx] = (unsigned char)(fmin(r, 1.0f) * 255);
            data[idx+1] = (unsigned char)(fmin(g, 1.0f) * 255);
            data[idx+2] = (unsigned char)(fmin(b, 1.0f) * 255);
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
