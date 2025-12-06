#pragma once

/// Pina Engine - Clear Pass
/// Simple pass that clears the render target

#include "../RenderPass.h"
#include "../RenderContext.h"

namespace Pina {

/// Pass that clears the render target
class PINA_API ClearPass : public RenderPass {
public:
    ClearPass() {
        name = "clear";
        needsSwap = false;  // Clearing doesn't need buffer swap
        clear = true;
        clearColor = Color(0.1f, 0.1f, 0.12f);
        clearDepth = true;
    }

    void execute(RenderContext& ctx) override {
        bindOutput(ctx);

        // Clear is handled by bindOutput when clear = true
        // Additional clearing can be done here if needed
    }

    /// Whether to clear the stencil buffer
    bool clearStencil = false;
};

} // namespace Pina
