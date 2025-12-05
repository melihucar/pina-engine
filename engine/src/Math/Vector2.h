#pragma once

/// Pina Engine - 2D Vector
/// Engine-branded 2D vector class wrapping glm::vec2

#include "../Core/Export.h"
#include <glm/vec2.hpp>
#include <cmath>

namespace Pina {

class PINA_API Vector2 {
public:
    float x, y;

    // Constructors
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float x, float y) : x(x), y(y) {}
    Vector2(float scalar) : x(scalar), y(scalar) {}
    Vector2(const glm::vec2& v) : x(v.x), y(v.y) {}

    // Conversion to glm
    operator glm::vec2() const { return glm::vec2(x, y); }

    // Array access
    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }

    // Arithmetic operators
    Vector2 operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
    Vector2 operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
    Vector2 operator*(const Vector2& v) const { return Vector2(x * v.x, y * v.y); }
    Vector2 operator/(const Vector2& v) const { return Vector2(x / v.x, y / v.y); }
    Vector2 operator*(float s) const { return Vector2(x * s, y * s); }
    Vector2 operator/(float s) const { return Vector2(x / s, y / s); }
    Vector2 operator-() const { return Vector2(-x, -y); }

    // Compound assignment
    Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
    Vector2& operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
    Vector2& operator*=(const Vector2& v) { x *= v.x; y *= v.y; return *this; }
    Vector2& operator/=(const Vector2& v) { x /= v.x; y /= v.y; return *this; }
    Vector2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vector2& operator/=(float s) { x /= s; y /= s; return *this; }

    // Comparison
    bool operator==(const Vector2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const Vector2& v) const { return !(*this == v); }

    // Length operations
    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSquared() const { return x * x + y * y; }
    Vector2 normalized() const {
        float len = length();
        return len > 0.0f ? *this / len : Vector2();
    }

    // Static operations
    static float dot(const Vector2& a, const Vector2& b) {
        return a.x * b.x + a.y * b.y;
    }

    static float distance(const Vector2& a, const Vector2& b) {
        return (b - a).length();
    }

    static Vector2 lerp(const Vector2& a, const Vector2& b, float t) {
        return a + (b - a) * t;
    }

    // Common vectors
    static Vector2 zero() { return Vector2(0.0f, 0.0f); }
    static Vector2 one() { return Vector2(1.0f, 1.0f); }
    static Vector2 up() { return Vector2(0.0f, 1.0f); }
    static Vector2 down() { return Vector2(0.0f, -1.0f); }
    static Vector2 left() { return Vector2(-1.0f, 0.0f); }
    static Vector2 right() { return Vector2(1.0f, 0.0f); }
};

// Scalar * Vector
inline Vector2 operator*(float s, const Vector2& v) {
    return v * s;
}

} // namespace Pina
