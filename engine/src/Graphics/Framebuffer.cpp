/// Pina Engine - Framebuffer Implementation

#include "Framebuffer.h"
#include "GraphicsDevice.h"
#include "OpenGL/GLFramebuffer.h"

namespace Pina {

UNIQUE<Framebuffer> Framebuffer::create(GraphicsDevice* device, const FramebufferSpec& spec) {
    if (!device) {
        return nullptr;
    }

    // Currently only OpenGL backend
    return MAKE_UNIQUE<GLFramebuffer>(spec);
}

} // namespace Pina
