#pragma once

/// Pina Engine - 4D Vector
/// Engine-branded 4D vector class wrapping glm::vec4

#include "../Core/Export.h"
#include <glm/vec4.hpp>
#include <cmath>

namespace Pina {

class PINA_API Vector4 {
public:
    float x, y, z, w;

    // Constructors
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
    Vector4(const glm::vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    // Conversion to glm
    operator glm::vec4() const { return glm::vec4(x, y, z, w); }

    // Array access
    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }

    // Arithmetic operators
    Vector4 operator+(const Vector4& v) const { return Vector4(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vector4 operator-(const Vector4& v) const { return Vector4(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vector4 operator*(const Vector4& v) const { return Vector4(x * v.x, y * v.y, z * v.z, w * v.w); }
    Vector4 operator/(const Vector4& v) const { return Vector4(x / v.x, y / v.y, z / v.z, w / v.w); }
    Vector4 operator*(float s) const { return Vector4(x * s, y * s, z * s, w * s); }
    Vector4 operator/(float s) const { return Vector4(x / s, y / s, z / s, w / s); }
    Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }

    // Compound assignment
    Vector4& operator+=(const Vector4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    Vector4& operator-=(const Vector4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    Vector4& operator*=(const Vector4& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
    Vector4& operator/=(const Vector4& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
    Vector4& operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    Vector4& operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }

    // Comparison
    bool operator==(const Vector4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const Vector4& v) const { return !(*this == v); }

    // Length operations
    float length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    float lengthSquared() const { return x * x + y * y + z * z + w * w; }
    Vector4 normalized() const {
        float len = length();
        return len > 0.0f ? *this / len : Vector4();
    }

    // Static operations
    static float dot(const Vector4& a, const Vector4& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    static Vector4 lerp(const Vector4& a, const Vector4& b, float t) {
        return a + (b - a) * t;
    }

    // Common vectors
    static Vector4 zero() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }
    static Vector4 one() { return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
};

// Scalar * Vector
inline Vector4 operator*(float s, const Vector4& v) {
    return v * s;
}

} // namespace Pina
