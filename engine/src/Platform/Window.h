#pragma once

/// Pina Engine - Window Subsystem
/// Abstract window interface for platform-specific implementations

#include "../Core/Export.h"
#include "../Core/Subsystem.h"
#include <string>
#include <functional>

namespace Pina {

/// Window configuration
struct PINA_API WindowConfig {
    int width = 1280;
    int height = 720;
    int minWidth = 0;      // 0 = no minimum
    int minHeight = 0;     // 0 = no minimum
    int maxWidth = 0;      // 0 = no maximum
    int maxHeight = 0;     // 0 = no maximum
    std::string title = "Pina Window";
    bool fullscreen = false;
    bool maximized = false;
    bool resizable = false;
};

/// Abstract window subsystem interface
/// Platform-specific implementations (Cocoa, Win32, X11) derive from this
class PINA_API Window : public Subsystem {
public:
    ~Window() override = default;

    // ========================================================================
    // Window Management
    // ========================================================================

    /// Create and show the window
    virtual bool create(const WindowConfig& config) = 0;

    /// Destroy the window
    virtual void destroy() = 0;

    /// Poll and process window events
    virtual void pollEvents() = 0;

    /// Check if window should close
    virtual bool shouldClose() const = 0;

    // ========================================================================
    // Native Handles
    // ========================================================================

    /// Get native window handle (NSWindow*, HWND, etc.)
    virtual void* getNativeHandle() const = 0;

    /// Get native view/content area handle (NSView*, etc.)
    virtual void* getNativeView() const = 0;

    // ========================================================================
    // Properties
    // ========================================================================

    /// Get window dimensions
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    /// Set window title
    virtual void setTitle(const std::string& title) = 0;

    // ========================================================================
    // Callbacks
    // ========================================================================

    using ResizeCallback = std::function<void(int width, int height)>;
    using CloseCallback = std::function<void()>;

    void setResizeCallback(ResizeCallback callback) { m_resizeCallback = callback; }
    void setCloseCallback(CloseCallback callback) { m_closeCallback = callback; }

    // ========================================================================
    // Factory
    // ========================================================================

    /// Creates platform-specific window (default implementation)
    static Window* createDefault();

protected:
    ResizeCallback m_resizeCallback;
    CloseCallback m_closeCallback;
};

} // namespace Pina
