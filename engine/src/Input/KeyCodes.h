#pragma once

/// Pina Engine - Key Codes
/// Platform-agnostic keyboard and mouse codes

#include "../Core/Export.h"
#include <cstdint>

namespace Pina {

/// Keyboard key codes (platform-agnostic)
enum class Key : uint16_t {
    Unknown = 0,

    // ========================================================================
    // Printable Keys
    // ========================================================================

    Space = 32,
    Apostrophe = 39,    // '
    Comma = 44,         // ,
    Minus = 45,         // -
    Period = 46,        // .
    Slash = 47,         // /

    // Numbers (top row)
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

    Semicolon = 59,     // ;
    Equal = 61,         // =

    // Letters
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    LeftBracket = 91,   // [
    Backslash = 92,     // '\'
    RightBracket = 93,  // ]
    GraveAccent = 96,   // `

    // ========================================================================
    // Function Keys
    // ========================================================================

    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,

    // F keys
    F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,

    // ========================================================================
    // Keypad
    // ========================================================================

    KP0 = 320, KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8, KP9,
    KPDecimal = 330,
    KPDivide = 331,
    KPMultiply = 332,
    KPSubtract = 333,
    KPAdd = 334,
    KPEnter = 335,
    KPEqual = 336,

    // ========================================================================
    // Modifiers
    // ========================================================================

    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,    // Command on macOS, Windows key on Windows
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348,

    // Maximum key value (for array sizing)
    MaxKey = 512
};

/// Mouse button codes
enum class MouseButton : uint8_t {
    Left = 0,
    Right = 1,
    Middle = 2,
    X1 = 3,         // Extra button 1 (back)
    X2 = 4,         // Extra button 2 (forward)

    MaxButton = 8
};

/// Key modifier flags
enum class KeyModifier : uint8_t {
    None     = 0,
    Shift    = 1 << 0,
    Control  = 1 << 1,
    Alt      = 1 << 2,
    Super    = 1 << 3,   // Command on macOS
    CapsLock = 1 << 4,
    NumLock  = 1 << 5
};

// Bitwise operators for KeyModifier
inline KeyModifier operator|(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline KeyModifier operator&(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline KeyModifier& operator|=(KeyModifier& a, KeyModifier b) {
    a = a | b;
    return a;
}

inline bool hasModifier(KeyModifier mods, KeyModifier flag) {
    return (static_cast<uint8_t>(mods) & static_cast<uint8_t>(flag)) != 0;
}

} // namespace Pina
