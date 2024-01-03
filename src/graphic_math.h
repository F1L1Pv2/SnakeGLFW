#pragma once

struct vector2 {
    float x, y;
    vector2(float x, float y) : x(x), y(y) {}
    float* value_ptr() {
        return &x;
    }

    vector2 operator+(const vector2& other) const {
        return vector2(x + other.x, y + other.y);
    }

    vector2 operator-(const vector2& other) const {
        return vector2(x - other.x, y - other.y);
    }

    bool operator==(const vector2& other) const {
        return x == other.x && y == other.y;
    }
};

struct vector3 {
    float x, y, z;
    vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct vertex {
    vector2 position, uv;
    vertex(vector2 position, vector2 uv) : position(position), uv(uv) {}
};

struct mat4 {

private:
    float 
        x11, x21, x31, x41,    //  x11 | x12 | x13 | x14
        x12, x22, x32, x42,    //  x21 | x22 | x23 | x24
        x13, x23, x33, x43,    //  x31 | x32 | x33 | x34
        x14, x24, x34, x44;    //  x41 | x42 | x43 | x44
public:

    mat4(float x11, float x12, float x13, float x14,
        float x21, float x22, float x23, float x24,
        float x31, float x32, float x33, float x34,
        float x41, float x42, float x43, float x44)
        :
        x11(x11), x12(x12), x13(x13), x14(x14),
        x21(x21), x22(x22), x23(x23), x24(x24),
        x31(x31), x32(x32), x33(x33), x34(x34),
        x41(x41), x42(x42), x43(x43), x44(x44)
    {}

    mat4() : 
        x11(0), x12(0), x13(0), x14(0),
        x21(0), x22(0), x23(0), x24(0),
        x31(0), x32(0), x33(0), x34(0),
        x41(0), x42(0), x43(0), x44(0)
    {}

    void translate(vector3 translation) {
        x14 += translation.x;
        x24 += translation.y;
        x34 += translation.z;
    }

    float* value_ptr() {
        return &x11;
    }

    static mat4 identity() {

        return mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    static mat4 ortho(float left, float right, float top, float bottom, float nearr, float farr) {
        return mat4(
            2 / (right - left), 0                 , 0                  , -(right + left) / (right - left),
            0                 , 2 / (top - bottom), 0                  , -(top + bottom) / (top - bottom),
            0                 , 0                 , -2 / (farr - nearr), -(farr + nearr) / (farr - nearr),
            0                 , 0                 , 0                  , 1
        );
        
    }

    static mat4 ortho2D(float left, float right, float top, float bottom) {
        return ortho(left, right, top, bottom, -1, 1);
    }
};