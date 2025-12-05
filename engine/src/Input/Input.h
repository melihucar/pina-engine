#pragma once

/// Pina Engine - Input Subsystem
/// Abstract input interface for keyboard and mouse

#include "../Core/Export.h"
#include "../Core/Subsystem.h"
#include "KeyCodes.h"
#include <glm/vec2.hpp>

namespace Pina {

class Window;

/// Abstract input subsystem interface
/// Provides polling-based input state queries
class PINA_API Input : public Subsystem {
public:
    ~Input() override = default;

    // ========================================================================
    // Keyboard State
    // ========================================================================

    /// Check if a key is currently held down
    virtual bool isKeyDown(Key key) const = 0;

    /// Check if a key was pressed this frame (first frame only)
    virtual bool isKeyPressed(Key key) const = 0;

    /// Check if a key was released this frame
    virtual bool isKeyReleased(Key key) const = 0;

    /// Get current key modifiers (Shift, Ctrl, Alt, Super)
    virtual KeyModifier getModifiers() const = 0;

    // ========================================================================
    // Mouse Button State
    // ========================================================================

    /// Check if a mouse button is currently held down
    virtual bool isMouseButtonDown(MouseButton button) const = 0;

    /// Check if a mouse button was pressed this frame
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;

    /// Check if a mouse button was released this frame
    virtual bool isMouseButtonReleased(MouseButton button) const = 0;

    // ========================================================================
    // Mouse Position & Movement
    // ========================================================================

    /// Get current mouse position in window coordinates (pixels)
    /// Origin is top-left corner
    virtual glm::vec2 getMousePosition() const = 0;

    /// Get mouse movement since last frame
    virtual glm::vec2 getMouseDelta() const = 0;

    /// Get scroll wheel delta since last frame
    /// x = horizontal scroll, y = vertical scroll
    virtual glm::vec2 getScrollDelta() const = 0;

    // ========================================================================
    // Mouse Capture & Visibility
    // ========================================================================

    /// Set mouse capture mode (for FPS-style controls)
    /// When captured, cursor is hidden and locked to center
    virtual void setMouseCaptured(bool captured) = 0;

    /// Check if mouse is captured
    virtual bool isMouseCaptured() const = 0;

    /// Set mouse cursor visibility
    virtual void setMouseVisible(bool visible) = 0;

    /// Check if mouse cursor is visible
    virtual bool isMouseVisible() const = 0;

    // ========================================================================
    // Window Focus
    // ========================================================================

    /// Check if the window has input focus
    virtual bool hasFocus() const = 0;

    // ========================================================================
    // Factory
    // ========================================================================

    /// Create platform-specific input subsystem (default implementation)
    static Input* createDefault(Window* window);

protected:
    /// Called at the end of frame to clear per-frame state
    /// (pressed/released flags, deltas)
    virtual void endFrame() = 0;

    friend class Application;
};

} // namespace Pina
