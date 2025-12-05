#pragma once

/// Pina Engine - UI Types
/// Type definitions and flags for the UI subsystem

#include "../Core/Export.h"
#include "../Math/Vector2.h"
#include "../Math/Color.h"
#include <cstdint>

namespace Pina {

/// UI configuration
struct PINA_API UIConfig {
    bool enableDocking = true;      // Enable docking support
    bool enableViewports = false;   // Enable multi-viewport (experimental)
    float fontSize = 16.0f;         // Base font size
};

/// Window flags for customizing window behavior
enum class UIWindowFlags : uint32_t {
    None            = 0,
    NoTitleBar      = 1 << 0,   // Disable title bar
    NoResize        = 1 << 1,   // Disable user resizing
    NoMove          = 1 << 2,   // Disable user moving the window
    NoScrollbar     = 1 << 3,   // Disable scrollbars
    NoCollapse      = 1 << 4,   // Disable user collapsing window
    AlwaysAutoResize = 1 << 5,  // Resize every frame to fit content
    NoBackground    = 1 << 6,   // Disable background
    NoSavedSettings = 1 << 7,   // Never save settings
    MenuBar         = 1 << 8,   // Has a menu bar
    HorizontalScrollbar = 1 << 9, // Allow horizontal scrollbar
};

// Bitwise operators for UIWindowFlags
inline UIWindowFlags operator|(UIWindowFlags a, UIWindowFlags b) {
    return static_cast<UIWindowFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline UIWindowFlags operator&(UIWindowFlags a, UIWindowFlags b) {
    return static_cast<UIWindowFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool operator!(UIWindowFlags f) {
    return static_cast<uint32_t>(f) == 0;
}

/// Input text flags
enum class UIInputTextFlags : uint32_t {
    None            = 0,
    CharsDecimal    = 1 << 0,   // Allow 0-9 . + -
    CharsHexadecimal = 1 << 1,  // Allow 0-9 a-f A-F
    CharsUppercase  = 1 << 2,   // Turn lowercase into uppercase
    CharsNoBlank    = 1 << 3,   // Filter out spaces, tabs
    AutoSelectAll   = 1 << 4,   // Select all on focus
    EnterReturnsTrue = 1 << 5,  // Return true on Enter
    Password        = 1 << 6,   // Password mode
    ReadOnly        = 1 << 7,   // Read-only mode
};

inline UIInputTextFlags operator|(UIInputTextFlags a, UIInputTextFlags b) {
    return static_cast<UIInputTextFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline UIInputTextFlags operator&(UIInputTextFlags a, UIInputTextFlags b) {
    return static_cast<UIInputTextFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

/// Tree node flags
enum class UITreeNodeFlags : uint32_t {
    None            = 0,
    Selected        = 1 << 0,   // Draw as selected
    Framed          = 1 << 1,   // Draw frame with background
    DefaultOpen     = 1 << 2,   // Default node to be open
    OpenOnArrow     = 1 << 3,   // Only open on arrow click
    Leaf            = 1 << 4,   // No collapsing, no arrow
    Bullet          = 1 << 5,   // Display a bullet instead of arrow
};

inline UITreeNodeFlags operator|(UITreeNodeFlags a, UITreeNodeFlags b) {
    return static_cast<UITreeNodeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline UITreeNodeFlags operator&(UITreeNodeFlags a, UITreeNodeFlags b) {
    return static_cast<UITreeNodeFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

/// Selectable flags
enum class UISelectableFlags : uint32_t {
    None            = 0,
    DontClosePopups = 1 << 0,   // Don't close parent popup on select
    SpanAllColumns  = 1 << 1,   // Span all columns in table
    AllowDoubleClick = 1 << 2,  // Generate press events on double click
};

inline UISelectableFlags operator|(UISelectableFlags a, UISelectableFlags b) {
    return static_cast<UISelectableFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline UISelectableFlags operator&(UISelectableFlags a, UISelectableFlags b) {
    return static_cast<UISelectableFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

} // namespace Pina
