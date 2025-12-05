#pragma once

/// Pina Engine - Color
/// RGBA color class with predefined colors

#include "../Core/Export.h"
#include <glm/vec4.hpp>
#include <cstdint>

namespace Pina {

class PINA_API Color {
public:
    float r, g, b, a;

    // Constructors
    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    Color(const glm::vec4& v) : r(v.x), g(v.y), b(v.z), a(v.w) {}

    // Conversion to glm
    operator glm::vec4() const { return glm::vec4(r, g, b, a); }

    // Array access
    float& operator[](int i) { return (&r)[i]; }
    const float& operator[](int i) const { return (&r)[i]; }

    // Comparison
    bool operator==(const Color& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
    bool operator!=(const Color& c) const { return !(*this == c); }

    // Color operations
    Color withAlpha(float alpha) const { return Color(r, g, b, alpha); }

    static Color lerp(const Color& a, const Color& b, float t) {
        return Color(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        );
    }

    // Create from 0-255 values
    static Color fromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    // Create from hex (0xRRGGBB or 0xRRGGBBAA)
    static Color fromHex(uint32_t hex) {
        if (hex <= 0xFFFFFF) {
            // RGB format
            return Color(
                ((hex >> 16) & 0xFF) / 255.0f,
                ((hex >> 8) & 0xFF) / 255.0f,
                (hex & 0xFF) / 255.0f,
                1.0f
            );
        } else {
            // RGBA format
            return Color(
                ((hex >> 24) & 0xFF) / 255.0f,
                ((hex >> 16) & 0xFF) / 255.0f,
                ((hex >> 8) & 0xFF) / 255.0f,
                (hex & 0xFF) / 255.0f
            );
        }
    }

    // Predefined colors
    static Color white() { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
    static Color black() { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
    static Color red() { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
    static Color green() { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
    static Color blue() { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
    static Color yellow() { return Color(1.0f, 1.0f, 0.0f, 1.0f); }
    static Color cyan() { return Color(0.0f, 1.0f, 1.0f, 1.0f); }
    static Color magenta() { return Color(1.0f, 0.0f, 1.0f, 1.0f); }
    static Color gray() { return Color(0.5f, 0.5f, 0.5f, 1.0f); }
    static Color darkGray() { return Color(0.25f, 0.25f, 0.25f, 1.0f); }
    static Color lightGray() { return Color(0.75f, 0.75f, 0.75f, 1.0f); }
    static Color orange() { return Color(1.0f, 0.5f, 0.0f, 1.0f); }
    static Color purple() { return Color(0.5f, 0.0f, 0.5f, 1.0f); }
    static Color transparent() { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
};

} // namespace Pina
