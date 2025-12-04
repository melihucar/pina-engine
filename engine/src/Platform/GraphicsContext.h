#pragma once

#include "../Core/Export.h"

namespace Pina {

class Window;

/// Graphics context configuration
struct PINA_API ContextConfig {
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

/// Abstract graphics context interface
/// Handles GPU context creation separate from window
/// This separation allows different backends (OpenGL, Metal, Vulkan) per platform
class PINA_API GraphicsContext {
public:
    virtual ~GraphicsContext() = default;

    /// Create graphics context for the given window
    virtual bool create(Window* window, const ContextConfig& config) = 0;

    /// Destroy the context
    virtual void destroy() = 0;

    /// Make this context current for rendering
    virtual void makeCurrent() = 0;

    /// Swap front/back buffers
    virtual void swapBuffers() = 0;

    /// Set vertical sync
    virtual void setVSync(bool enabled) = 0;

    /// Update context after window resize
    virtual void update() = 0;

    /// Get the backend type
    virtual GraphicsBackend getBackend() const = 0;

    /// Factory method - creates context based on backend type
    static GraphicsContext* create(GraphicsBackend backend);
};

} // namespace Pina
