#pragma once

/// Pina Engine - Render Compositor
/// Manages render pass chain with ping-pong buffers (inspired by Three.js EffectComposer)

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include "RenderPass.h"
#include "RenderContext.h"
#include "Framebuffer.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace Pina {

// Forward declarations
class GraphicsDevice;
class Scene;
class Camera;
class VertexArray;
class VertexBuffer;
class Shader;

/// Manages render pass chain
class PINA_API RenderCompositor {
public:
    explicit RenderCompositor(GraphicsDevice* device);
    ~RenderCompositor();

    // No copying
    RenderCompositor(const RenderCompositor&) = delete;
    RenderCompositor& operator=(const RenderCompositor&) = delete;

    // ========================================================================
    // Pass Management
    // ========================================================================

    /// Add a pass to the end of the chain
    void addPass(UNIQUE<RenderPass> pass);

    /// Insert a pass at a specific index
    void insertPass(UNIQUE<RenderPass> pass, size_t index);

    /// Remove a pass by name
    void removePass(const std::string& name);

    /// Get a pass by name
    RenderPass* getPass(const std::string& name);

    /// Get a pass by name with type casting
    template<typename T>
    T* getPass(const std::string& name) {
        return dynamic_cast<T*>(getPass(name));
    }

    /// Get all passes
    const std::vector<UNIQUE<RenderPass>>& getPasses() const { return m_passes; }

    /// Get number of passes
    size_t getPassCount() const { return m_passes.size(); }

    // ========================================================================
    // Named Render Targets
    // ========================================================================

    /// Create a named render target
    /// @param name Target name (e.g., "shadowMap", "gBuffer")
    /// @param spec Framebuffer specification
    void createRenderTarget(const std::string& name, const FramebufferSpec& spec);

    /// Get a named render target
    Framebuffer* getRenderTarget(const std::string& name);

    /// Remove a named render target
    void removeRenderTarget(const std::string& name);

    // ========================================================================
    // Execution
    // ========================================================================

    /// Execute all enabled passes
    /// @param scene Scene to render
    /// @param camera Camera to use
    /// @param deltaTime Time since last frame
    void render(Scene* scene, Camera* camera, float deltaTime);

    /// Execute all enabled passes with custom shaders
    /// @param scene Scene to render
    /// @param camera Camera to use
    /// @param deltaTime Time since last frame
    /// @param standardShader Standard Blinn-Phong shader
    /// @param pbrShader PBR shader
    /// @param shadowShader Shadow depth shader
    void render(Scene* scene, Camera* camera, float deltaTime,
                Shader* standardShader, Shader* pbrShader, Shader* shadowShader);

    /// Handle viewport resize
    void resize(int width, int height);

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Set the clear color for the final output
    void setClearColor(const Color& color) { m_clearColor = color; }

    /// Get the graphics device
    GraphicsDevice* getDevice() const { return m_device; }

    /// Get viewport dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    void swapBuffers();
    bool isLastEnabledPass(size_t index) const;
    void createPingPongBuffers();
    void createFullscreenQuad();
    void initializeContext();

    GraphicsDevice* m_device;
    std::vector<UNIQUE<RenderPass>> m_passes;

    // Named render targets (shadow maps, G-buffer, etc.)
    std::unordered_map<std::string, UNIQUE<Framebuffer>> m_namedTargets;
    std::unordered_map<std::string, Framebuffer*> m_namedTargetPtrs; // For RenderContext

    // Ping-pong buffers for post-processing
    UNIQUE<Framebuffer> m_pingBuffer;
    UNIQUE<Framebuffer> m_pongBuffer;
    Framebuffer* m_readBuffer = nullptr;
    Framebuffer* m_writeBuffer = nullptr;

    // Fullscreen quad for post-processing
    UNIQUE<VertexArray> m_quadVAO;
    UNIQUE<VertexBuffer> m_quadVBO;
    UNIQUE<Shader> m_blitShader;

    // Render context (reused each frame)
    RenderContext m_context;

    // Viewport dimensions
    int m_width = 1280;
    int m_height = 720;

    // Clear color
    Color m_clearColor = Color(0.1f, 0.1f, 0.12f);

    // Frame counter
    uint64_t m_frameNumber = 0;
    float m_totalTime = 0.0f;
};

} // namespace Pina
