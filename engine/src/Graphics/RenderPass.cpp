/// Pina Engine - Render Pass Implementation

#include "RenderPass.h"
#include "RenderContext.h"
#include "Framebuffer.h"
#include "GraphicsDevice.h"

namespace Pina {

void RenderPass::bindOutput(RenderContext& ctx) {
    if (renderToScreen) {
        // Bind default framebuffer (screen)
        if (ctx.writeBuffer) {
            ctx.writeBuffer->unbind();
        }
        ctx.device->setViewport(0, 0, ctx.viewportWidth, ctx.viewportHeight);
    } else if (ctx.writeBuffer) {
        // Bind write buffer
        ctx.writeBuffer->bind();
    }

    // Clear if requested
    if (clear) {
        if (clearDepth) {
            ctx.device->clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        } else {
            // Clear color only - need to handle this case
            // For now, just do full clear
            ctx.device->clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        }
    }
}

} // namespace Pina
