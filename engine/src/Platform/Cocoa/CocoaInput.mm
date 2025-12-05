/// Pina Engine - Cocoa Input Implementation

#import "CocoaInput.h"
#import "../Window.h"
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>  // For kVK_ virtual key codes

namespace Pina {

CocoaInput::CocoaInput(Window* window)
    : m_window(window)
{
    if (window) {
        m_nsWindow = (__bridge NSWindow*)window->getNativeHandle();
        m_nsView = (__bridge NSView*)window->getNativeView();
    }
}

CocoaInput::~CocoaInput() = default;

void CocoaInput::initialize() {
    // Initial state already zeroed by array initialization
}

void CocoaInput::update(float deltaTime) {
    (void)deltaTime;

    // Calculate mouse delta from position change
    m_mouseDelta = m_mousePosition - m_mousePreviousPosition;
    m_mousePreviousPosition = m_mousePosition;
}

void CocoaInput::shutdown() {
    // Restore cursor if hidden
    if (!m_mouseVisible) {
        [NSCursor unhide];
    }
    if (m_mouseCaptured) {
        CGAssociateMouseAndMouseCursorPosition(true);
    }
}

void CocoaInput::endFrame() {
    // Copy current state to previous for edge detection
    m_keyPreviousState = m_keyCurrentState;
    m_mousePreviousState = m_mouseCurrentState;

    // Reset per-frame deltas
    m_scrollDelta = glm::vec2(0.0f);
}

// ============================================================================
// Keyboard
// ============================================================================

bool CocoaInput::isKeyDown(Key key) const {
    size_t index = static_cast<size_t>(key);
    if (index >= KEY_COUNT) return false;
    return m_keyCurrentState[index];
}

bool CocoaInput::isKeyPressed(Key key) const {
    size_t index = static_cast<size_t>(key);
    if (index >= KEY_COUNT) return false;
    return m_keyCurrentState[index] && !m_keyPreviousState[index];
}

bool CocoaInput::isKeyReleased(Key key) const {
    size_t index = static_cast<size_t>(key);
    if (index >= KEY_COUNT) return false;
    return !m_keyCurrentState[index] && m_keyPreviousState[index];
}

KeyModifier CocoaInput::getModifiers() const {
    return m_modifiers;
}

// ============================================================================
// Mouse Buttons
// ============================================================================

bool CocoaInput::isMouseButtonDown(MouseButton button) const {
    size_t index = static_cast<size_t>(button);
    if (index >= BUTTON_COUNT) return false;
    return m_mouseCurrentState[index];
}

bool CocoaInput::isMouseButtonPressed(MouseButton button) const {
    size_t index = static_cast<size_t>(button);
    if (index >= BUTTON_COUNT) return false;
    return m_mouseCurrentState[index] && !m_mousePreviousState[index];
}

bool CocoaInput::isMouseButtonReleased(MouseButton button) const {
    size_t index = static_cast<size_t>(button);
    if (index >= BUTTON_COUNT) return false;
    return !m_mouseCurrentState[index] && m_mousePreviousState[index];
}

// ============================================================================
// Mouse Position
// ============================================================================

glm::vec2 CocoaInput::getMousePosition() const {
    return m_mousePosition;
}

glm::vec2 CocoaInput::getMouseDelta() const {
    return m_mouseDelta;
}

glm::vec2 CocoaInput::getScrollDelta() const {
    return m_scrollDelta;
}

// ============================================================================
// Mouse Capture
// ============================================================================

void CocoaInput::setMouseCaptured(bool captured) {
    if (m_mouseCaptured == captured) return;
    m_mouseCaptured = captured;

    @autoreleasepool {
        if (captured) {
            CGAssociateMouseAndMouseCursorPosition(false);
            // Center mouse in window
            if (m_nsWindow) {
                NSRect frame = [m_nsWindow frame];
                CGFloat screenHeight = CGDisplayBounds(CGMainDisplayID()).size.height;
                CGPoint center = CGPointMake(
                    frame.origin.x + frame.size.width / 2,
                    screenHeight - (frame.origin.y + frame.size.height / 2)
                );
                CGWarpMouseCursorPosition(center);
            }
        } else {
            CGAssociateMouseAndMouseCursorPosition(true);
        }
    }
}

bool CocoaInput::isMouseCaptured() const {
    return m_mouseCaptured;
}

void CocoaInput::setMouseVisible(bool visible) {
    if (m_mouseVisible == visible) return;
    m_mouseVisible = visible;

    @autoreleasepool {
        if (visible) {
            [NSCursor unhide];
        } else {
            [NSCursor hide];
        }
    }
}

bool CocoaInput::isMouseVisible() const {
    return m_mouseVisible;
}

// ============================================================================
// Focus
// ============================================================================

bool CocoaInput::hasFocus() const {
    return m_hasFocus;
}

// ============================================================================
// Event Processing (called by CocoaWindow)
// ============================================================================

void CocoaInput::processKeyDown(unsigned short keyCode) {
    Key key = translateKeyCode(keyCode);
    size_t index = static_cast<size_t>(key);
    if (index < KEY_COUNT) {
        m_keyCurrentState[index] = true;
    }
}

void CocoaInput::processKeyUp(unsigned short keyCode) {
    Key key = translateKeyCode(keyCode);
    size_t index = static_cast<size_t>(key);
    if (index < KEY_COUNT) {
        m_keyCurrentState[index] = false;
    }
}

void CocoaInput::processModifiersChanged(unsigned int nsModifiers) {
    m_modifiers = translateModifiers(nsModifiers);
}

void CocoaInput::processMouseDown(MouseButton button) {
    size_t index = static_cast<size_t>(button);
    if (index < BUTTON_COUNT) {
        m_mouseCurrentState[index] = true;
    }
}

void CocoaInput::processMouseUp(MouseButton button) {
    size_t index = static_cast<size_t>(button);
    if (index < BUTTON_COUNT) {
        m_mouseCurrentState[index] = false;
    }
}

void CocoaInput::processMouseMove(float x, float y) {
    m_mousePosition.x = x;
    m_mousePosition.y = y;
}

void CocoaInput::processScroll(float deltaX, float deltaY) {
    m_scrollDelta.x += deltaX;
    m_scrollDelta.y += deltaY;
}

void CocoaInput::processFocusChange(bool hasFocus) {
    m_hasFocus = hasFocus;

    // Reset all keys when losing focus to prevent stuck keys
    if (!hasFocus) {
        m_keyCurrentState.fill(false);
        m_mouseCurrentState.fill(false);
    }
}

// ============================================================================
// Key Code Translation
// ============================================================================

Key CocoaInput::translateKeyCode(unsigned short keyCode) {
    // macOS virtual key codes from Carbon/HIToolbox/Events.h
    switch (keyCode) {
        // Letters
        case kVK_ANSI_A: return Key::A;
        case kVK_ANSI_B: return Key::B;
        case kVK_ANSI_C: return Key::C;
        case kVK_ANSI_D: return Key::D;
        case kVK_ANSI_E: return Key::E;
        case kVK_ANSI_F: return Key::F;
        case kVK_ANSI_G: return Key::G;
        case kVK_ANSI_H: return Key::H;
        case kVK_ANSI_I: return Key::I;
        case kVK_ANSI_J: return Key::J;
        case kVK_ANSI_K: return Key::K;
        case kVK_ANSI_L: return Key::L;
        case kVK_ANSI_M: return Key::M;
        case kVK_ANSI_N: return Key::N;
        case kVK_ANSI_O: return Key::O;
        case kVK_ANSI_P: return Key::P;
        case kVK_ANSI_Q: return Key::Q;
        case kVK_ANSI_R: return Key::R;
        case kVK_ANSI_S: return Key::S;
        case kVK_ANSI_T: return Key::T;
        case kVK_ANSI_U: return Key::U;
        case kVK_ANSI_V: return Key::V;
        case kVK_ANSI_W: return Key::W;
        case kVK_ANSI_X: return Key::X;
        case kVK_ANSI_Y: return Key::Y;
        case kVK_ANSI_Z: return Key::Z;

        // Numbers
        case kVK_ANSI_0: return Key::Num0;
        case kVK_ANSI_1: return Key::Num1;
        case kVK_ANSI_2: return Key::Num2;
        case kVK_ANSI_3: return Key::Num3;
        case kVK_ANSI_4: return Key::Num4;
        case kVK_ANSI_5: return Key::Num5;
        case kVK_ANSI_6: return Key::Num6;
        case kVK_ANSI_7: return Key::Num7;
        case kVK_ANSI_8: return Key::Num8;
        case kVK_ANSI_9: return Key::Num9;

        // Function keys
        case kVK_F1: return Key::F1;
        case kVK_F2: return Key::F2;
        case kVK_F3: return Key::F3;
        case kVK_F4: return Key::F4;
        case kVK_F5: return Key::F5;
        case kVK_F6: return Key::F6;
        case kVK_F7: return Key::F7;
        case kVK_F8: return Key::F8;
        case kVK_F9: return Key::F9;
        case kVK_F10: return Key::F10;
        case kVK_F11: return Key::F11;
        case kVK_F12: return Key::F12;
        case kVK_F13: return Key::F13;
        case kVK_F14: return Key::F14;
        case kVK_F15: return Key::F15;
        case kVK_F16: return Key::F16;
        case kVK_F17: return Key::F17;
        case kVK_F18: return Key::F18;
        case kVK_F19: return Key::F19;
        case kVK_F20: return Key::F20;

        // Special keys
        case kVK_Escape: return Key::Escape;
        case kVK_Return: return Key::Enter;
        case kVK_Tab: return Key::Tab;
        case kVK_Delete: return Key::Backspace;
        case kVK_ForwardDelete: return Key::Delete;
        case kVK_Space: return Key::Space;

        // Arrow keys
        case kVK_LeftArrow: return Key::Left;
        case kVK_RightArrow: return Key::Right;
        case kVK_UpArrow: return Key::Up;
        case kVK_DownArrow: return Key::Down;

        // Navigation
        case kVK_Home: return Key::Home;
        case kVK_End: return Key::End;
        case kVK_PageUp: return Key::PageUp;
        case kVK_PageDown: return Key::PageDown;

        // Modifiers
        case kVK_Shift: return Key::LeftShift;
        case kVK_RightShift: return Key::RightShift;
        case kVK_Control: return Key::LeftControl;
        case kVK_RightControl: return Key::RightControl;
        case kVK_Option: return Key::LeftAlt;
        case kVK_RightOption: return Key::RightAlt;
        case kVK_Command: return Key::LeftSuper;
        case kVK_RightCommand: return Key::RightSuper;
        case kVK_CapsLock: return Key::CapsLock;

        // Punctuation
        case kVK_ANSI_Semicolon: return Key::Semicolon;
        case kVK_ANSI_Equal: return Key::Equal;
        case kVK_ANSI_Comma: return Key::Comma;
        case kVK_ANSI_Minus: return Key::Minus;
        case kVK_ANSI_Period: return Key::Period;
        case kVK_ANSI_Slash: return Key::Slash;
        case kVK_ANSI_Grave: return Key::GraveAccent;
        case kVK_ANSI_LeftBracket: return Key::LeftBracket;
        case kVK_ANSI_RightBracket: return Key::RightBracket;
        case kVK_ANSI_Backslash: return Key::Backslash;
        case kVK_ANSI_Quote: return Key::Apostrophe;

        // Keypad
        case kVK_ANSI_Keypad0: return Key::KP0;
        case kVK_ANSI_Keypad1: return Key::KP1;
        case kVK_ANSI_Keypad2: return Key::KP2;
        case kVK_ANSI_Keypad3: return Key::KP3;
        case kVK_ANSI_Keypad4: return Key::KP4;
        case kVK_ANSI_Keypad5: return Key::KP5;
        case kVK_ANSI_Keypad6: return Key::KP6;
        case kVK_ANSI_Keypad7: return Key::KP7;
        case kVK_ANSI_Keypad8: return Key::KP8;
        case kVK_ANSI_Keypad9: return Key::KP9;
        case kVK_ANSI_KeypadDecimal: return Key::KPDecimal;
        case kVK_ANSI_KeypadDivide: return Key::KPDivide;
        case kVK_ANSI_KeypadMultiply: return Key::KPMultiply;
        case kVK_ANSI_KeypadMinus: return Key::KPSubtract;
        case kVK_ANSI_KeypadPlus: return Key::KPAdd;
        case kVK_ANSI_KeypadEnter: return Key::KPEnter;
        case kVK_ANSI_KeypadEquals: return Key::KPEqual;

        default: return Key::Unknown;
    }
}

KeyModifier CocoaInput::translateModifiers(unsigned int nsModifiers) {
    KeyModifier mods = KeyModifier::None;

    if (nsModifiers & NSEventModifierFlagShift) {
        mods = mods | KeyModifier::Shift;
    }
    if (nsModifiers & NSEventModifierFlagControl) {
        mods = mods | KeyModifier::Control;
    }
    if (nsModifiers & NSEventModifierFlagOption) {
        mods = mods | KeyModifier::Alt;
    }
    if (nsModifiers & NSEventModifierFlagCommand) {
        mods = mods | KeyModifier::Super;
    }
    if (nsModifiers & NSEventModifierFlagCapsLock) {
        mods = mods | KeyModifier::CapsLock;
    }
    if (nsModifiers & NSEventModifierFlagNumericPad) {
        mods = mods | KeyModifier::NumLock;
    }

    return mods;
}

// ============================================================================
// Factory
// ============================================================================

Input* Input::createDefault(Window* window) {
    return new CocoaInput(window);
}

} // namespace Pina
