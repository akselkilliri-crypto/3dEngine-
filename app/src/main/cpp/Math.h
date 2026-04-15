#ifndef MATH_H
#define MATH_H

#include <cmath>
#include <algorithm>

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 cross(const Vec3& other) const {
        return Vec3(y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x);
    }
    Vec3 normalized() const {
        float len = std::sqrt(x*x + y*y + z*z);
        if (len > 0) return Vec3(x/len, y/len, z/len);
        return *this;
    }
    float dot(const Vec3& other) const { return x*other.x + y*other.y + z*other.z; }
};

struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(1) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Mat4 {
    float data[16];

    Mat4() { for(int i=0;i<16;++i) data[i]=0; }
    static Mat4 identity() {
        Mat4 m;
        m.data[0]=1; m.data[5]=1; m.data[10]=1; m.data[15]=1;
        return m;
    }
    Mat4 operator*(const Mat4& other) const {
        Mat4 r;
        for(int col=0; col<4; ++col) {
            for(int row=0; row<4; ++row) {
                float sum = 0;
                for(int k=0; k<4; ++k) {
                    sum += data[k*4+row] * other.data[col*4+k];
                }
                r.data[col*4+row] = sum;
            }
        }
        return r;
    }
    Vec4 operator*(const Vec4& v) const {
        Vec4 r;
        r.x = data[0]*v.x + data[4]*v.y + data[8]*v.z  + data[12]*v.w;
        r.y = data[1]*v.x + data[5]*v.y + data[9]*v.z  + data[13]*v.w;
        r.z = data[2]*v.x + data[6]*v.y + data[10]*v.z + data[14]*v.w;
        r.w = data[3]*v.x + data[7]*v.y + data[11]*v.z + data[15]*v.w;
        return r;
    }
    static Mat4 perspective(float fovy, float aspect, float near, float far) {
        Mat4 m;
        float f = 1.0f / std::tan(fovy / 2.0f);
        m.data[0] = f / aspect;
        m.data[5] = f;
        m.data[10] = (far + near) / (near - far);
        m.data[11] = -1.0f;
        m.data[14] = (2.0f * far * near) / (near - far);
        return m;
    }
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).normalized();
        Vec3 s = f.cross(up).normalized();
        Vec3 u = s.cross(f);

        Mat4 m;
        m.data[0] = s.x;  m.data[4] = s.y;  m.data[8]  = s.z;  m.data[12] = -s.dot(eye);
        m.data[1] = u.x;  m.data[5] = u.y;  m.data[9]  = u.z;  m.data[13] = -u.dot(eye);
        m.data[2] = -f.x; m.data[6] = -f.y; m.data[10] = -f.z; m.data[14] = f.dot(eye);
        m.data[3] = 0;    m.data[7] = 0;    m.data[11] = 0;    m.data[15] = 1.0f;
        return m;
    }

    static Mat4 inverse(const Mat4& m) {
        float inv[16];
        inv[0] = m.data[5]*m.data[10]*m.data[15] - m.data[5]*m.data[11]*m.data[14] - m.data[9]*m.data[6]*m.data[15] + m.data[9]*m.data[7]*m.data[14] + m.data[13]*m.data[6]*m.data[11] - m.data[13]*m.data[7]*m.data[10];
        inv[4] = -m.data[4]*m.data[10]*m.data[15] + m.data[4]*m.data[11]*m.data[14] + m.data[8]*m.data[6]*m.data[15] - m.data[8]*m.data[7]*m.data[14] - m.data[12]*m.data[6]*m.data[11] + m.data[12]*m.data[7]*m.data[10];
        inv[8] = m.data[4]*m.data[9]*m.data[15] - m.data[4]*m.data[11]*m.data[13] - m.data[8]*m.data[5]*m.data[15] + m.data[8]*m.data[7]*m.data[13] + m.data[12]*m.data[5]*m.data[11] - m.data[12]*m.data[7]*m.data[9];
        inv[12] = -m.data[4]*m.data[9]*m.data[14] + m.data[4]*m.data[10]*m.data[13] + m.data[8]*m.data[5]*m.data[14] - m.data[8]*m.data[6]*m.data[13] - m.data[12]*m.data[5]*m.data[10] + m.data[12]*m.data[6]*m.data[9];
        inv[1] = -m.data[1]*m.data[10]*m.data[15] + m.data[1]*m.data[11]*m.data[14] + m.data[9]*m.data[2]*m.data[15] - m.data[9]*m.data[3]*m.data[14] - m.data[13]*m.data[2]*m.data[11] + m.data[13]*m.data[3]*m.data[10];
        inv[5] = m.data[0]*m.data[10]*m.data[15] - m.data[0]*m.data[11]*m.data[14] - m.data[8]*m.data[2]*m.data[15] + m.data[8]*m.data[3]*m.data[14] + m.data[12]*m.data[2]*m.data[11] - m.data[12]*m.data[3]*m.data[10];
        inv[9] = -m.data[0]*m.data[9]*m.data[15] + m.data[0]*m.data[11]*m.data[13] + m.data[8]*m.data[1]*m.data[15] - m.data[8]*m.data[3]*m.data[13] - m.data[12]*m.data[1]*m.data[11] + m.data[12]*m.data[3]*m.data[9];
        inv[13] = m.data[0]*m.data[9]*m.data[14] - m.data[0]*m.data[10]*m.data[13] - m.data[8]*m.data[1]*m.data[14] + m.data[8]*m.data[2]*m.data[13] + m.data[12]*m.data[1]*m.data[10] - m.data[12]*m.data[2]*m.data[9];
        inv[2] = m.data[1]*m.data[6]*m.data[15] - m.data[1]*m.data[7]*m.data[14] - m.data[5]*m.data[2]*m.data[15] + m.data[5]*m.data[3]*m.data[14] + m.data[13]*m.data[2]*m.data[7] - m.data[13]*m.data[3]*m.data[6];
        inv[6] = -m.data[0]*m.data[6]*m.data[15] + m.data[0]*m.data[7]*m.data[14] + m.data[4]*m.data[2]*m.data[15] - m.data[4]*m.data[3]*m.data[14] - m.data[12]*m.data[2]*m.data[7] + m.data[12]*m.data[3]*m.data[6];
        inv[10] = m.data[0]*m.data[5]*m.data[15] - m.data[0]*m.data[7]*m.data[13] - m.data[4]*m.data[1]*m.data[15] + m.data[4]*m.data[3]*m.data[13] + m.data[12]*m.data[1]*m.data[7] - m.data[12]*m.data[3]*m.data[5];
        inv[14] = -m.data[0]*m.data[5]*m.data[14] + m.data[0]*m.data[6]*m.data[13] + m.data[4]*m.data[1]*m.data[14] - m.data[4]*m.data[2]*m.data[13] - m.data[12]*m.data[1]*m.data[6] + m.data[12]*m.data[2]*m.data[5];
        inv[3] = -m.data[1]*m.data[6]*m.data[11] + m.data[1]*m.data[7]*m.data[10] + m.data[5]*m.data[2]*m.data[11] - m.data[5]*m.data[3]*m.data[10] - m.data[9]*m.data[2]*m.data[7] + m.data[9]*m.data[3]*m.data[6];
        inv[7] = m.data[0]*m.data[6]*m.data[11] - m.data[0]*m.data[7]*m.data[10] - m.data[4]*m.data[2]*m.data[11] + m.data[4]*m.data[3]*m.data[10] + m.data[8]*m.data[2]*m.data[7] - m.data[8]*m.data[3]*m.data[6];
        inv[11] = -m.data[0]*m.data[5]*m.data[11] + m.data[0]*m.data[7]*m.data[9] + m.data[4]*m.data[1]*m.data[11] - m.data[4]*m.data[3]*m.data[9] - m.data[8]*m.data[1]*m.data[7] + m.data[8]*m.data[3]*m.data[5];
        inv[15] = m.data[0]*m.data[5]*m.data[10] - m.data[0]*m.data[6]*m.data[9] - m.data[4]*m.data[1]*m.data[10] + m.data[4]*m.data[2]*m.data[9] + m.data[8]*m.data[1]*m.data[6] - m.data[8]*m.data[2]*m.data[5];

        float det = m.data[0]*inv[0] + m.data[1]*inv[4] + m.data[2]*inv[8] + m.data[3]*inv[12];
        if (det == 0) return Mat4::identity();
        det = 1.0f / det;

        Mat4 res;
        for (int i = 0; i < 16; i++) res.data[i] = inv[i] * det;
        return res;
    }
};

#endif
