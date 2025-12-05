#pragma once

/// Pina Engine - Cocoa Input Implementation
/// macOS-specific input handling using NSEvent

#include "../../Input/Input.h"
#include <array>

#ifdef __OBJC__
@class NSWindow;
@class NSView;
#else
typedef void NSWindow;
typedef void NSView;
#endif

namespace Pina {

class Window;
class EventDispatcher;

/// macOS Cocoa input implementation
class CocoaInput : public Input {
public:
    explicit CocoaInput(Window* window);
    ~CocoaInput() override;

    // Subsystem lifecycle
    void initialize() override;
    void update(float deltaTime) override;
    void shutdown() override;

    // ========================================================================
    // Keyboard
    // ========================================================================

    bool isKeyDown(Key key) const override;
    bool isKeyPressed(Key key) const override;
    bool isKeyReleased(Key key) const override;
    KeyModifier getModifiers() const override;

    // ========================================================================
    // Mouse Buttons
    // ========================================================================

    bool isMouseButtonDown(MouseButton button) const override;
    bool isMouseButtonPressed(MouseButton button) const override;
    bool isMouseButtonReleased(MouseButton button) const override;

    // ========================================================================
    // Mouse Position
    // ========================================================================

    glm::vec2 getMousePosition() const override;
    glm::vec2 getMouseDelta() const override;
    glm::vec2 getScrollDelta() const override;

    // ========================================================================
    // Mouse Capture
    // ========================================================================

    void setMouseCaptured(bool captured) override;
    bool isMouseCaptured() const override;
    void setMouseVisible(bool visible) override;
    bool isMouseVisible() const override;

    // ========================================================================
    // Focus
    // ========================================================================

    bool hasFocus() const override;

    // ========================================================================
    // Event Processing (called by CocoaWindow)
    // ========================================================================

    void processKeyDown(unsigned short keyCode);
    void processKeyUp(unsigned short keyCode);
    void processModifiersChanged(unsigned int nsModifiers);
    void processMouseDown(MouseButton button);
    void processMouseUp(MouseButton button);
    void processMouseMove(float x, float y);
    void processScroll(float deltaX, float deltaY);
    void processFocusChange(bool hasFocus);

    // ========================================================================
    // Key Translation
    // ========================================================================

    static Key translateKeyCode(unsigned short keyCode);
    static KeyModifier translateModifiers(unsigned int nsModifiers);

    // ========================================================================
    // Event System Integration
    // ========================================================================

    /// Set the event dispatcher for emitting input events
    void setEventDispatcher(EventDispatcher* dispatcher);

protected:
    void endFrame() override;

private:
    // Owning window
    Window* m_window = nullptr;
    NSWindow* m_nsWindow = nullptr;
    NSView* m_nsView = nullptr;

    // Keyboard state
    static constexpr size_t KEY_COUNT = static_cast<size_t>(Key::MaxKey);
    std::array<bool, KEY_COUNT> m_keyCurrentState{};
    std::array<bool, KEY_COUNT> m_keyPreviousState{};
    KeyModifier m_modifiers = KeyModifier::None;

    // Mouse button state
    static constexpr size_t BUTTON_COUNT = static_cast<size_t>(MouseButton::MaxButton);
    std::array<bool, BUTTON_COUNT> m_mouseCurrentState{};
    std::array<bool, BUTTON_COUNT> m_mousePreviousState{};

    // Mouse position
    glm::vec2 m_mousePosition{0.0f, 0.0f};
    glm::vec2 m_mousePreviousPosition{0.0f, 0.0f};
    glm::vec2 m_mouseDelta{0.0f, 0.0f};
    glm::vec2 m_scrollDelta{0.0f, 0.0f};

    // Mouse capture state
    bool m_mouseCaptured = false;
    bool m_mouseVisible = true;

    // Focus state
    bool m_hasFocus = true;

    // Event dispatcher for emitting events
    EventDispatcher* m_eventDispatcher = nullptr;
};

} // namespace Pina
