#pragma once

/// Pina Engine - UI Subsystem
/// Abstract interface for the UI subsystem
/// Hides the underlying UI library (currently ImGui)

#include "../Core/Subsystem.h"
#include "../Core/Export.h"
#include "UITypes.h"

namespace Pina {

class Window;
class Graphics;

/// Abstract UI subsystem interface
/// Extends Subsystem for lifecycle integration with Application
class PINA_API UISubsystem : public Subsystem {
public:
    UISubsystem() = default;
    virtual ~UISubsystem() = default;

    /// Create and initialize the UI context
    /// @param window The window for input/rendering
    /// @param graphics The graphics context
    /// @param config UI configuration options
    /// @return true if successful
    virtual bool create(Window* window, Graphics* graphics, const UIConfig& config = {}) = 0;

    /// Destroy the UI context
    virtual void destroy() = 0;

    /// Begin a new UI frame (call before rendering UI)
    virtual void beginFrame() = 0;

    /// End the UI frame and render (call after rendering UI)
    virtual void endFrame() = 0;

    /// Check if UI wants to capture keyboard input
    /// Use this to disable game input when typing in UI
    virtual bool wantsCaptureKeyboard() const = 0;

    /// Check if UI wants to capture mouse input
    /// Use this to disable game input when interacting with UI
    virtual bool wantsCaptureMouse() const = 0;

    /// Show the built-in demo window (useful for testing)
    /// @param open Optional pointer to control window visibility
    virtual void showDemoWindow(bool* open = nullptr) = 0;

    /// Factory method to create the default UI implementation
    /// @return Platform-specific UI implementation (currently ImGuiUI)
    static UISubsystem* createDefault();
};

// Alias for convenience
using UI = UISubsystem;

} // namespace Pina
