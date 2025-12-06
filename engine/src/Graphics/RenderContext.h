#pragma once

/// Pina Engine - Render Context
/// Context passed to each render pass during execution

#include "../Core/Export.h"
#include "Framebuffer.h"
#include <string>
#include <unordered_map>
#include <functional>

namespace Pina {

// Forward declarations
class GraphicsDevice;
class Scene;
class Camera;
class LightManager;
class Shader;
class VertexArray;
class VertexBuffer;

/// Context passed to each render pass
struct PINA_API RenderContext {
    // ========================================================================
    // Core Resources
    // ========================================================================

    /// Graphics device for rendering operations
    GraphicsDevice* device = nullptr;

    /// Current scene being rendered
    Scene* scene = nullptr;

    /// Active camera for the scene
    Camera* camera = nullptr;

    /// Light manager for the scene
    LightManager* lights = nullptr;

    // ========================================================================
    // Frame Info
    // ========================================================================

    /// Time since last frame in seconds
    float deltaTime = 0.0f;

    /// Total elapsed time in seconds
    float totalTime = 0.0f;

    /// Current frame number
    uint64_t frameNumber = 0;

    // ========================================================================
    // Viewport
    // ========================================================================

    /// Viewport width in pixels
    int viewportWidth = 1280;

    /// Viewport height in pixels
    int viewportHeight = 720;

    // ========================================================================
    // Ping-Pong Buffers
    // ========================================================================

    /// Input buffer from previous pass (read from this)
    Framebuffer* readBuffer = nullptr;

    /// Output buffer for this pass (write to this)
    Framebuffer* writeBuffer = nullptr;

    // ========================================================================
    // Named Render Targets
    // ========================================================================

    /// Get a named render target (e.g., "shadowMap", "gBuffer")
    /// @param name Target name
    /// @return Framebuffer pointer, or nullptr if not found
    Framebuffer* getTarget(const std::string& name) const {
        auto it = namedTargets->find(name);
        return (it != namedTargets->end()) ? it->second : nullptr;
    }

    /// Get texture ID from a named target's color attachment
    /// Convenience method for binding as input texture
    uint32_t getTextureID(const std::string& name, int attachmentIndex = 0) const {
        Framebuffer* fb = getTarget(name);
        return fb ? fb->getColorAttachmentID(attachmentIndex) : 0;
    }

    /// Get depth texture ID from a named target
    uint32_t getDepthTextureID(const std::string& name) const {
        Framebuffer* fb = getTarget(name);
        return fb ? fb->getDepthAttachmentID() : 0;
    }

    /// Pointer to named targets map (owned by compositor)
    std::unordered_map<std::string, Framebuffer*>* namedTargets = nullptr;

    // ========================================================================
    // Fullscreen Quad
    // ========================================================================

    /// Draw a fullscreen quad (for post-processing)
    /// Assumes appropriate shader is already bound
    std::function<void()> drawFullscreenQuad;

    // ========================================================================
    // Built-in Shaders (for convenience)
    // ========================================================================

    /// Standard Blinn-Phong shader
    Shader* standardShader = nullptr;

    /// PBR shader
    Shader* pbrShader = nullptr;

    /// Shadow depth shader
    Shader* shadowShader = nullptr;

    /// Fullscreen blit shader
    Shader* blitShader = nullptr;
};

} // namespace Pina
