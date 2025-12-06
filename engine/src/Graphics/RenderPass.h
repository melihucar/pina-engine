#pragma once

/// Pina Engine - Render Pass Base Class
/// Abstract base class for all render passes (inspired by Three.js Pass)

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include "../Math/Color.h"
#include <string>

namespace Pina {

// Forward declaration
struct RenderContext;

/// Abstract base class for all render passes
class PINA_API RenderPass {
public:
    virtual ~RenderPass() = default;

    // ========================================================================
    // Execution
    // ========================================================================

    /// Execute this render pass
    /// @param ctx Render context with all necessary resources
    virtual void execute(RenderContext& ctx) = 0;

    /// Called when render targets are resized
    /// Override to resize pass-specific resources (e.g., blur buffers)
    virtual void resize(int width, int height) { (void)width; (void)height; }

    /// Called once when pass is added to compositor
    /// Override to create pass-specific resources
    virtual void initialize(RenderContext& ctx) { (void)ctx; }

    /// Called when pass is removed from compositor
    /// Override to cleanup pass-specific resources
    virtual void cleanup() {}

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Pass name (for debugging and lookup)
    std::string name;

    /// Whether this pass is enabled
    /// Disabled passes are skipped during rendering
    bool enabled = true;

    /// Whether to swap read/write buffers after this pass
    /// Set to false for passes that write to named targets (e.g., shadow pass)
    bool needsSwap = true;

    /// Force output to screen instead of write buffer
    /// Typically true for the last pass in the chain
    bool renderToScreen = false;

    /// Whether to clear the render target before rendering
    bool clear = false;

    /// Color to clear with (if clear is true)
    Color clearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);

    /// Clear depth buffer (if clear is true)
    bool clearDepth = true;

protected:
    /// Helper to bind the correct output target
    /// Binds screen (FBO 0) if renderToScreen, otherwise writeBuffer
    void bindOutput(RenderContext& ctx);
};

} // namespace Pina
