#pragma once

/// Pina Engine - Math utilities
/// General math functions similar to Unity's Mathf class

#include "../Core/Export.h"
#include <cmath>
#include <algorithm>
#include <limits>

namespace Pina {

/// Collection of common math functions and constants
class PINA_API Mathf {
public:
    // Constants
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float TAU = PI * 2.0f;
    static constexpr float E = 2.71828182845904523536f;
    static constexpr float DEG_TO_RAD = PI / 180.0f;
    static constexpr float RAD_TO_DEG = 180.0f / PI;
    static constexpr float EPSILON = 1e-6f;
    static constexpr float INFINITY_VALUE = std::numeric_limits<float>::infinity();
    static constexpr float NEG_INFINITY_VALUE = -std::numeric_limits<float>::infinity();

    // Basic operations
    static float abs(float value) { return std::abs(value); }
    static float sign(float value) { return value < 0.0f ? -1.0f : (value > 0.0f ? 1.0f : 0.0f); }
    static float floor(float value) { return std::floor(value); }
    static float ceil(float value) { return std::ceil(value); }
    static float round(float value) { return std::round(value); }
    static float frac(float value) { return value - std::floor(value); }

    // Clamping and range
    static float clamp(float value, float min, float max) {
        return std::min(std::max(value, min), max);
    }
    static float clamp01(float value) {
        return clamp(value, 0.0f, 1.0f);
    }
    static float min(float a, float b) { return std::min(a, b); }
    static float max(float a, float b) { return std::max(a, b); }
    static float min(float a, float b, float c) { return std::min({a, b, c}); }
    static float max(float a, float b, float c) { return std::max({a, b, c}); }

    // Power and roots
    static float pow(float base, float exponent) { return std::pow(base, exponent); }
    static float sqrt(float value) { return std::sqrt(value); }
    static float exp(float value) { return std::exp(value); }
    static float log(float value) { return std::log(value); }
    static float log10(float value) { return std::log10(value); }

    // Trigonometry (radians)
    static float sin(float radians) { return std::sin(radians); }
    static float cos(float radians) { return std::cos(radians); }
    static float tan(float radians) { return std::tan(radians); }
    static float asin(float value) { return std::asin(value); }
    static float acos(float value) { return std::acos(value); }
    static float atan(float value) { return std::atan(value); }
    static float atan2(float y, float x) { return std::atan2(y, x); }

    // Angle conversions
    static float degToRad(float degrees) { return degrees * DEG_TO_RAD; }
    static float radToDeg(float radians) { return radians * RAD_TO_DEG; }

    // Interpolation
    static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    static float lerpUnclamped(float a, float b, float t) {
        return a + (b - a) * t;
    }
    static float inverseLerp(float a, float b, float value) {
        if (abs(b - a) < EPSILON) return 0.0f;
        return (value - a) / (b - a);
    }
    static float remap(float value, float fromMin, float fromMax, float toMin, float toMax) {
        float t = inverseLerp(fromMin, fromMax, value);
        return lerp(toMin, toMax, t);
    }

    // Smoothing
    static float smoothstep(float edge0, float edge1, float x) {
        float t = clamp01((x - edge0) / (edge1 - edge0));
        return t * t * (3.0f - 2.0f * t);
    }
    static float smootherstep(float edge0, float edge1, float x) {
        float t = clamp01((x - edge0) / (edge1 - edge0));
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    // Damping
    static float moveTowards(float current, float target, float maxDelta) {
        if (abs(target - current) <= maxDelta) return target;
        return current + sign(target - current) * maxDelta;
    }
    static float smoothDamp(float current, float target, float& velocity, float smoothTime, float deltaTime) {
        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        float change = current - target;
        float temp = (velocity + omega * change) * deltaTime;
        velocity = (velocity - omega * temp) * exp;
        return target + (change + temp) * exp;
    }

    // Angle operations
    static float deltaAngle(float current, float target) {
        float delta = repeat(target - current, 360.0f);
        if (delta > 180.0f) delta -= 360.0f;
        return delta;
    }
    static float repeat(float t, float length) {
        return clamp(t - floor(t / length) * length, 0.0f, length);
    }
    static float pingPong(float t, float length) {
        t = repeat(t, length * 2.0f);
        return length - abs(t - length);
    }
    static float lerpAngle(float a, float b, float t) {
        float delta = repeat(b - a, 360.0f);
        if (delta > 180.0f) delta -= 360.0f;
        return a + delta * clamp01(t);
    }

    // Comparison
    static bool approximately(float a, float b, float epsilon = EPSILON) {
        return abs(a - b) < epsilon;
    }
    static bool isFinite(float value) { return std::isfinite(value); }
    static bool isNaN(float value) { return std::isnan(value); }
    static bool isInfinity(float value) { return std::isinf(value); }

    // Integer operations
    static int floorToInt(float value) { return static_cast<int>(std::floor(value)); }
    static int ceilToInt(float value) { return static_cast<int>(std::ceil(value)); }
    static int roundToInt(float value) { return static_cast<int>(std::round(value)); }
    static bool isPowerOfTwo(int value) { return value > 0 && (value & (value - 1)) == 0; }
    static int nextPowerOfTwo(int value) {
        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        return value + 1;
    }
};

} // namespace Pina
