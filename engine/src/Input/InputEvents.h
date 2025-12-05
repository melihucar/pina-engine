#pragma once

/// Pina Engine - Input Event Types
/// Keyboard, mouse, and window events

#include "../Core/Event.h"
#include "KeyCodes.h"
#include <glm/vec2.hpp>

namespace Pina {

// ============================================================================
// Keyboard Events
// ============================================================================

/// Key pressed event (includes repeat detection)
class PINA_API KeyPressedEvent : public EventBase<KeyPressedEvent> {
public:
    KeyPressedEvent(Key k, KeyModifier mods, bool repeat = false)
        : key(k), modifiers(mods), isRepeat(repeat) {}

    Key key;
    KeyModifier modifiers;
    bool isRepeat;

    EventCategory getCategories() const override {
        return EventCategory::Input | EventCategory::Keyboard;
    }

    const char* getName() const override { return "KeyPressedEvent"; }
};

/// Key released event
class PINA_API KeyReleasedEvent : public EventBase<KeyReleasedEvent> {
public:
    KeyReleasedEvent(Key k, KeyModifier mods)
        : key(k), modifiers(mods) {}

    Key key;
    KeyModifier modifiers;

    EventCategory getCategories() const override {
        return EventCategory::Input | EventCategory::Keyboard;
    }

    const char* getName() const override { return "KeyReleasedEvent"; }
};

// ============================================================================
// Mouse Button Events
// ============================================================================

/// Mouse button pressed event
class PINA_API MouseButtonPressedEvent : public EventBase<MouseButtonPressedEvent> {
public:
    MouseButtonPressedEvent(MouseButton btn, glm::vec2 pos, KeyModifier mods)
        : button(btn), position(pos), modifiers(mods) {}

    MouseButton button;
    glm::vec2 position;
    KeyModifier modifiers;

    EventCategory getCategories() const override {
        return EventCategory::Input | EventCategory::Mouse;
    }

    const char* getName() const override { return "MouseButtonPressedEvent"; }
};

/// Mouse button released event
class PINA_API MouseButtonReleasedEvent : public EventBase<MouseButtonReleasedEvent> {
public:
    MouseButtonReleasedEvent(MouseButton btn, glm::vec2 pos, KeyModifier mods)
        : button(btn), position(pos), modifiers(mods) {}

    MouseButton button;
    glm::vec2 position;
    KeyModifier modifiers;

    EventCategory getCategories() const override {
        return EventCategory::Input | EventCategory::Mouse;
    }

    const char* getName() const override { return "MouseButtonReleasedEvent"; }
};

// ============================================================================
// Mouse Movement Events
// ============================================================================

/// Mouse moved event
class PINA_API MouseMovedEvent : public EventBase<MouseMovedEvent> {
public:
    MouseMovedEvent(glm::vec2 pos, glm::vec2 d)
        : position(pos), delta(d) {}

    glm::vec2 position;
    glm::vec2 delta;

    EventCategory getCategories() const override {
        return EventCategory::Input | EventCategory::Mouse;
    }

    const char* getName() const override { return "MouseMovedEvent"; }
};

/// Mouse scroll event
class PINA_API MouseScrolledEvent : public EventBase<MouseScrolledEvent> {
public:
    MouseScrolledEvent(glm::vec2 scrollDelta, glm::vec2 pos)
        : delta(scrollDelta), position(pos) {}

    glm::vec2 delta;      // x = horizontal, y = vertical
    glm::vec2 position;   // Mouse position when scrolled

    EventCategory getCategories() const override {
        return EventCategory::Input | EventCategory::Mouse;
    }

    const char* getName() const override { return "MouseScrolledEvent"; }
};

// ============================================================================
// Window Events
// ============================================================================

/// Window resize event
class PINA_API WindowResizeEvent : public EventBase<WindowResizeEvent> {
public:
    WindowResizeEvent(int w, int h) : width(w), height(h) {}

    int width;
    int height;

    EventCategory getCategories() const override {
        return EventCategory::Window;
    }

    const char* getName() const override { return "WindowResizeEvent"; }
};

/// Window close event
class PINA_API WindowCloseEvent : public EventBase<WindowCloseEvent> {
public:
    EventCategory getCategories() const override {
        return EventCategory::Window;
    }

    const char* getName() const override { return "WindowCloseEvent"; }
};

/// Window focus change event
class PINA_API WindowFocusEvent : public EventBase<WindowFocusEvent> {
public:
    explicit WindowFocusEvent(bool focused) : hasFocus(focused) {}

    bool hasFocus;

    EventCategory getCategories() const override {
        return EventCategory::Window;
    }

    const char* getName() const override { return "WindowFocusEvent"; }
};

} // namespace Pina
