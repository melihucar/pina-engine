/// Pina Engine - Render Compositor Implementation

#include "RenderCompositor.h"
#include "GraphicsDevice.h"
#include "Shader.h"
#include "Buffer.h"
#include "VertexLayout.h"
#include "../Scene/Scene.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Lighting/LightManager.h"
#include <iostream>

namespace Pina {

// ============================================================================
// Fullscreen Quad Shader
// ============================================================================

static const char* BLIT_VERTEX_SHADER = R"(
#version 410 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

static const char* BLIT_FRAGMENT_SHADER = R"(
#version 410 core

uniform sampler2D uInputTexture;

in vec2 vTexCoord;
out vec4 FragColor;

void main() {
    FragColor = texture(uInputTexture, vTexCoord);
}
)";

// ============================================================================
// Constructor / Destructor
// ============================================================================

RenderCompositor::RenderCompositor(GraphicsDevice* device)
    : m_device(device)
{
    createPingPongBuffers();
    createFullscreenQuad();
    initializeContext();
}

RenderCompositor::~RenderCompositor() {
    // Cleanup passes
    for (auto& pass : m_passes) {
        pass->cleanup();
    }
}

// ============================================================================
// Pass Management
// ============================================================================

void RenderCompositor::addPass(UNIQUE<RenderPass> pass) {
    pass->initialize(m_context);
    m_passes.push_back(std::move(pass));
}

void RenderCompositor::insertPass(UNIQUE<RenderPass> pass, size_t index) {
    if (index > m_passes.size()) {
        index = m_passes.size();
    }
    pass->initialize(m_context);
    m_passes.insert(m_passes.begin() + static_cast<ptrdiff_t>(index), std::move(pass));
}

void RenderCompositor::removePass(const std::string& name) {
    for (auto it = m_passes.begin(); it != m_passes.end(); ++it) {
        if ((*it)->name == name) {
            (*it)->cleanup();
            m_passes.erase(it);
            return;
        }
    }
}

RenderPass* RenderCompositor::getPass(const std::string& name) {
    for (auto& pass : m_passes) {
        if (pass->name == name) {
            return pass.get();
        }
    }
    return nullptr;
}

// ============================================================================
// Named Render Targets
// ============================================================================

void RenderCompositor::createRenderTarget(const std::string& name, const FramebufferSpec& spec) {
    auto fb = Framebuffer::create(m_device, spec);
    if (fb) {
        m_namedTargetPtrs[name] = fb.get();
        m_namedTargets[name] = std::move(fb);
    }
}

Framebuffer* RenderCompositor::getRenderTarget(const std::string& name) {
    auto it = m_namedTargets.find(name);
    return (it != m_namedTargets.end()) ? it->second.get() : nullptr;
}

void RenderCompositor::removeRenderTarget(const std::string& name) {
    m_namedTargetPtrs.erase(name);
    m_namedTargets.erase(name);
}

// ============================================================================
// Execution
// ============================================================================

void RenderCompositor::render(Scene* scene, Camera* camera, float deltaTime) {
    render(scene, camera, deltaTime, nullptr, nullptr, nullptr);
}

void RenderCompositor::render(Scene* scene, Camera* camera, float deltaTime,
                               Shader* standardShader, Shader* pbrShader, Shader* shadowShader) {
    if (!scene || !camera) {
        return;
    }

    // Update context
    m_context.scene = scene;
    m_context.camera = camera;
    m_context.lights = &scene->getLightManager();
    m_context.deltaTime = deltaTime;
    m_context.totalTime = m_totalTime;
    m_context.frameNumber = m_frameNumber;

    // Set shaders in context
    m_context.standardShader = standardShader;
    m_context.pbrShader = pbrShader;
    m_context.shadowShader = shadowShader;

    // Reset ping-pong buffers
    m_readBuffer = m_pingBuffer.get();
    m_writeBuffer = m_pongBuffer.get();
    m_context.readBuffer = m_readBuffer;
    m_context.writeBuffer = m_writeBuffer;

    // Execute all enabled passes
    for (size_t i = 0; i < m_passes.size(); ++i) {
        RenderPass* pass = m_passes[i].get();

        if (!pass->enabled) {
            continue;
        }

        // Update context buffers
        m_context.readBuffer = m_readBuffer;
        m_context.writeBuffer = m_writeBuffer;

        // Check if this is the last enabled pass
        if (isLastEnabledPass(i)) {
            pass->renderToScreen = true;
        }

        // Execute the pass
        pass->execute(m_context);

        // Swap buffers if needed
        if (pass->needsSwap && !pass->renderToScreen) {
            swapBuffers();
        }

        // Reset renderToScreen (it was set temporarily)
        if (isLastEnabledPass(i)) {
            pass->renderToScreen = false;
        }
    }

    // Update frame counter
    m_frameNumber++;
    m_totalTime += deltaTime;
}

void RenderCompositor::resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    m_width = width;
    m_height = height;

    // Update context
    m_context.viewportWidth = width;
    m_context.viewportHeight = height;

    // Resize ping-pong buffers
    if (m_pingBuffer) {
        m_pingBuffer->resize(width, height);
    }
    if (m_pongBuffer) {
        m_pongBuffer->resize(width, height);
    }

    // Resize named targets (except fixed-size ones like shadow maps)
    // Note: Shadow maps typically have fixed resolution, so we skip them
    // You might want to add a flag to FramebufferSpec to control this

    // Notify passes
    for (auto& pass : m_passes) {
        pass->resize(width, height);
    }
}

// ============================================================================
// Private Methods
// ============================================================================

void RenderCompositor::swapBuffers() {
    std::swap(m_readBuffer, m_writeBuffer);
}

bool RenderCompositor::isLastEnabledPass(size_t index) const {
    for (size_t i = index + 1; i < m_passes.size(); ++i) {
        if (m_passes[i]->enabled) {
            return false;
        }
    }
    return true;
}

void RenderCompositor::createPingPongBuffers() {
    FramebufferSpec spec;
    spec.width = m_width;
    spec.height = m_height;
    spec.colorAttachments = { TextureFormat::RGBA16F }; // HDR format
    spec.depthAttachment = TextureFormat::Depth24Stencil8;

    m_pingBuffer = Framebuffer::create(m_device, spec);
    m_pongBuffer = Framebuffer::create(m_device, spec);

    m_readBuffer = m_pingBuffer.get();
    m_writeBuffer = m_pongBuffer.get();
}

void RenderCompositor::createFullscreenQuad() {
    // Fullscreen quad vertices (position + texcoord)
    // NDC coordinates: (-1,-1) to (1,1)
    float quadVertices[] = {
        // Position     // TexCoord
        -1.0f,  1.0f,   0.0f, 1.0f,   // Top-left
        -1.0f, -1.0f,   0.0f, 0.0f,   // Bottom-left
         1.0f, -1.0f,   1.0f, 0.0f,   // Bottom-right

        -1.0f,  1.0f,   0.0f, 1.0f,   // Top-left
         1.0f, -1.0f,   1.0f, 0.0f,   // Bottom-right
         1.0f,  1.0f,   1.0f, 1.0f    // Top-right
    };

    // Create VBO
    m_quadVBO = m_device->createVertexBuffer(quadVertices, sizeof(quadVertices));

    // Create VAO with vertex layout: position (2 floats) + texcoord (2 floats)
    VertexLayout layout;
    layout.push("aPosition", ShaderDataType::Float2);  // location 0: vec2 position
    layout.push("aTexCoord", ShaderDataType::Float2);  // location 1: vec2 texcoord

    m_quadVAO = m_device->createVertexArray();
    m_quadVAO->addVertexBuffer(m_quadVBO.get(), layout);

    // Create blit shader
    m_blitShader = m_device->createShader();
    if (!m_blitShader->load(BLIT_VERTEX_SHADER, BLIT_FRAGMENT_SHADER)) {
        std::cerr << "RenderCompositor: Failed to create blit shader" << std::endl;
    }
}

void RenderCompositor::initializeContext() {
    m_context.device = m_device;
    m_context.viewportWidth = m_width;
    m_context.viewportHeight = m_height;
    m_context.namedTargets = &m_namedTargetPtrs;
    m_context.blitShader = m_blitShader.get();

    // Setup fullscreen quad draw function
    m_context.drawFullscreenQuad = [this]() {
        m_quadVAO->bind();
        m_device->draw(m_quadVAO.get(), 6);
    };
}

} // namespace Pina
