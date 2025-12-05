#pragma once

/// Pina Engine - UI Widgets
/// RAII-based widget classes that abstract the underlying UI library
///
/// Usage:
///   using namespace Pina::UI;
///   Window window("My Window");
///   if (window) {
///       Text("Hello");
///       if (Button("Click")) { ... }
///   }

#include "../Core/Export.h"
#include "../Math/Vector2.h"
#include "../Math/Color.h"
#include "UITypes.h"
#include <cstddef>

namespace Pina::Widgets {

// ============================================================================
// Container Widgets (RAII - auto begin/end)
// ============================================================================

/// Window container - automatically calls End() on destruction
class PINA_API Window {
public:
    /// Create a window
    /// @param title Window title
    /// @param open Optional pointer to control visibility (window has close button if not null)
    /// @param flags Window behavior flags
    Window(const char* title, bool* open = nullptr, UIWindowFlags flags = UIWindowFlags::None);
    ~Window();

    /// Check if window content should be rendered
    bool isVisible() const { return m_visible; }
    operator bool() const { return m_visible; }

    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

private:
    bool m_visible;
};

/// Child region container - scrollable sub-region within a window
class PINA_API Child {
public:
    /// Create a child region
    /// @param id Unique identifier
    /// @param size Size (0,0 = auto)
    /// @param border Draw border
    Child(const char* id, const Vector2& size = Vector2(), bool border = false);
    ~Child();

    bool isVisible() const { return m_visible; }
    operator bool() const { return m_visible; }

    Child(const Child&) = delete;
    Child& operator=(const Child&) = delete;

private:
    bool m_visible;
};

/// Tree node container - collapsible tree structure
class PINA_API TreeNode {
public:
    /// Create a tree node
    /// @param label Node label
    /// @param flags Tree node flags
    TreeNode(const char* label, UITreeNodeFlags flags = UITreeNodeFlags::None);
    ~TreeNode();

    bool isOpen() const { return m_open; }
    operator bool() const { return m_open; }

    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;

private:
    bool m_open;
};

/// Menu bar container
class PINA_API MenuBar {
public:
    MenuBar();
    ~MenuBar();

    bool isVisible() const { return m_visible; }
    operator bool() const { return m_visible; }

    MenuBar(const MenuBar&) = delete;
    MenuBar& operator=(const MenuBar&) = delete;

private:
    bool m_visible;
};

/// Menu container (inside menu bar)
class PINA_API Menu {
public:
    /// Create a menu
    /// @param label Menu label
    Menu(const char* label);
    ~Menu();

    bool isOpen() const { return m_open; }
    operator bool() const { return m_open; }

    Menu(const Menu&) = delete;
    Menu& operator=(const Menu&) = delete;

private:
    bool m_open;
};

// ============================================================================
// Display Widgets (render immediately)
// ============================================================================

/// Text display
class PINA_API Text {
public:
    /// Display text
    Text(const char* text);

    /// Display colored text
    Text(const Color& color, const char* text);
};

/// Separator line
class PINA_API Separator {
public:
    Separator();
};

/// Place next widget on same line
class PINA_API SameLine {
public:
    /// Continue on same line
    /// @param offsetX Offset from start of line (0 = right after previous)
    /// @param spacing Spacing from previous widget (-1 = default)
    SameLine(float offsetX = 0.0f, float spacing = -1.0f);
};

/// Add spacing
class PINA_API Spacing {
public:
    Spacing();
};

/// Add indent
class PINA_API Indent {
public:
    /// Push indent
    /// @param width Indent width (0 = default)
    Indent(float width = 0.0f);
    ~Indent();

    Indent(const Indent&) = delete;
    Indent& operator=(const Indent&) = delete;

private:
    float m_width;
};

// ============================================================================
// Interactive Widgets (check result after construction)
// ============================================================================

/// Button widget
class PINA_API Button {
public:
    /// Create a button
    /// @param label Button text
    /// @param size Button size (0,0 = auto)
    Button(const char* label, const Vector2& size = Vector2());

    bool clicked() const { return m_clicked; }
    operator bool() const { return m_clicked; }

private:
    bool m_clicked;
};

/// Small button (no frame padding)
class PINA_API SmallButton {
public:
    SmallButton(const char* label);

    bool clicked() const { return m_clicked; }
    operator bool() const { return m_clicked; }

private:
    bool m_clicked;
};

/// Checkbox widget
class PINA_API Checkbox {
public:
    /// Create a checkbox
    /// @param label Label text
    /// @param value Pointer to boolean value
    Checkbox(const char* label, bool* value);

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

/// Float slider widget
class PINA_API SliderFloat {
public:
    /// Create a float slider
    /// @param label Label text
    /// @param value Pointer to float value
    /// @param min Minimum value
    /// @param max Maximum value
    /// @param format Display format (e.g., "%.3f")
    SliderFloat(const char* label, float* value, float min, float max, const char* format = "%.3f");

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

/// Int slider widget
class PINA_API SliderInt {
public:
    /// Create an int slider
    /// @param label Label text
    /// @param value Pointer to int value
    /// @param min Minimum value
    /// @param max Maximum value
    SliderInt(const char* label, int* value, int min, int max);

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

/// Float drag widget
class PINA_API DragFloat {
public:
    /// Create a float drag control
    /// @param label Label text
    /// @param value Pointer to float value
    /// @param speed Drag speed
    /// @param min Minimum value (optional)
    /// @param max Maximum value (optional)
    DragFloat(const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f);

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

/// Float input widget
class PINA_API InputFloat {
public:
    /// Create a float input field
    /// @param label Label text
    /// @param value Pointer to float value
    /// @param step Step amount (0 = no buttons)
    InputFloat(const char* label, float* value, float step = 0.0f);

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

/// Int input widget
class PINA_API InputInt {
public:
    /// Create an int input field
    /// @param label Label text
    /// @param value Pointer to int value
    InputInt(const char* label, int* value);

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

/// Text input widget
class PINA_API InputText {
public:
    /// Create a text input field
    /// @param label Label text
    /// @param buf Text buffer
    /// @param bufSize Buffer size
    /// @param flags Input flags
    InputText(const char* label, char* buf, size_t bufSize, UIInputTextFlags flags = UIInputTextFlags::None);

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

/// Collapsing header widget
class PINA_API CollapsingHeader {
public:
    /// Create a collapsing header
    /// @param label Header text
    /// @param flags Tree node flags (use DefaultOpen to start expanded)
    CollapsingHeader(const char* label, UITreeNodeFlags flags = UITreeNodeFlags::None);

    bool isOpen() const { return m_open; }
    operator bool() const { return m_open; }

private:
    bool m_open;
};

/// Selectable item widget
class PINA_API Selectable {
public:
    /// Create a selectable item
    /// @param label Item text
    /// @param selected Whether currently selected
    /// @param flags Selectable flags
    /// @param size Size (0,0 = auto)
    Selectable(const char* label, bool selected = false, UISelectableFlags flags = UISelectableFlags::None, const Vector2& size = Vector2());

    bool clicked() const { return m_clicked; }
    operator bool() const { return m_clicked; }

private:
    bool m_clicked;
};

/// Menu item widget
class PINA_API MenuItem {
public:
    /// Create a menu item
    /// @param label Item text
    /// @param shortcut Keyboard shortcut text (display only)
    /// @param selected Whether currently selected (for toggle items)
    /// @param enabled Whether item is enabled
    MenuItem(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);

    bool clicked() const { return m_clicked; }
    operator bool() const { return m_clicked; }

private:
    bool m_clicked;
};

/// Color edit widget (RGBA)
class PINA_API ColorEdit {
public:
    /// Create a color editor
    /// @param label Label text
    /// @param color Pointer to Color value
    ColorEdit(const char* label, Color* color);

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

/// Combo box widget
class PINA_API Combo {
public:
    /// Create a combo box
    /// @param label Label text
    /// @param currentItem Pointer to current item index
    /// @param items Null-separated items string (e.g., "Item1\0Item2\0Item3\0")
    Combo(const char* label, int* currentItem, const char* items);

    bool changed() const { return m_changed; }

private:
    bool m_changed;
};

// ============================================================================
// Utility Functions
// ============================================================================

/// Check if last widget is hovered
PINA_API bool isItemHovered();

/// Check if last widget was clicked
PINA_API bool isItemClicked();

/// Check if last widget is active (being dragged, etc.)
PINA_API bool isItemActive();

/// Get available content region size
PINA_API Vector2 getContentRegionAvail();

/// Get window size
PINA_API Vector2 getWindowSize();

/// Set next window position
PINA_API void setNextWindowPos(const Vector2& pos);

/// Set next window size
PINA_API void setNextWindowSize(const Vector2& size);

} // namespace Pina::Widgets
