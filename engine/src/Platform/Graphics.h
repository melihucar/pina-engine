#pragma once

/// Pina Engine - Graphics Subsystem
/// Abstract graphics context interface for rendering backends

#include "../Core/Export.h"
#include "../Core/Subsystem.h"

namespace Pina {

class Window;

/// Graphics configuration
struct PINA_API GraphicsConfig {
    int majorVersion = 4;
    int minorVersion = 1;  // macOS max is 4.1
    bool coreProfile = true;
    bool vsync = true;
    int samples = 0;       // MSAA samples (0 = disabled)
    int depthBits = 24;    // Depth buffer bits
    int stencilBits = 8;   // Stencil buffer bits
};

/// Graphics backend type
enum class GraphicsBackend {
    OpenGL,
    Metal,      // Future
    Vulkan,     // Future
    DirectX12   // Future (Windows only)
};

/// Abstract graphics subsystem interface
/// Handles GPU context creation separate from window
/// This separation allows different backends (OpenGL, Metal, Vulkan) per platform
class PINA_API Graphics : public Subsystem {
public:
    ~Graphics() override = default;

    // ========================================================================
    // Context Management
    // ========================================================================

    /// Create graphics context for the given window
    virtual bool create(Window* window, const GraphicsConfig& config) = 0;

    /// Destroy the context
    virtual void destroy() = 0;

    /// Make this context current for rendering
    virtual void makeCurrent() = 0;

    /// Swap front/back buffers
    virtual void swapBuffers() = 0;

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Set vertical sync
    virtual void setVSync(bool enabled) = 0;

    /// Update context after window resize (call when window is resized)
    virtual void updateContext() = 0;

    /// Get the backend type
    virtual GraphicsBackend getBackend() const = 0;

    // ========================================================================
    // Factory
    // ========================================================================

    /// Creates default graphics context based on backend type
    static Graphics* createDefault(GraphicsBackend backend);
};

// Backwards compatibility alias
using GraphicsContext = Graphics;
using ContextConfig = GraphicsConfig;

} // namespace Pina
