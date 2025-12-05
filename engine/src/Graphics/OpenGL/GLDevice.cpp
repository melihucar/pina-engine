/// Pina Engine - OpenGL Graphics Device Implementation

#include "GLDevice.h"
#include "GLShader.h"
#include "GLBuffer.h"
#include "GLTexture.h"
#include <iostream>

namespace Pina {

GLDevice::GLDevice() {
    // Log OpenGL info
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Enable depth testing by default
    glEnable(GL_DEPTH_TEST);
}

GLDevice::~GLDevice() {
    // Nothing to clean up - all resources are managed by their own classes
}

// ============================================================================
// Resource Creation
// ============================================================================

UNIQUE<Shader> GLDevice::createShader() {
    return MAKE_UNIQUE<GLShader>();
}

UNIQUE<VertexBuffer> GLDevice::createVertexBuffer(const void* data, size_t size) {
    return MAKE_UNIQUE<GLVertexBuffer>(data, size);
}

UNIQUE<IndexBuffer> GLDevice::createIndexBuffer(const uint32_t* indices, uint32_t count) {
    return MAKE_UNIQUE<GLIndexBuffer>(indices, count);
}

UNIQUE<VertexArray> GLDevice::createVertexArray() {
    return MAKE_UNIQUE<GLVertexArray>();
}

UNIQUE<Texture> GLDevice::createTexture(const unsigned char* data,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t channels) {
    return MAKE_UNIQUE<GLTexture>(data, width, height, channels);
}

// ============================================================================
// Frame Lifecycle
// ============================================================================

void GLDevice::beginFrame() {
    // Nothing special needed for OpenGL
}

void GLDevice::endFrame() {
    // Nothing special needed for OpenGL
    // Buffer swap is handled by GraphicsContext
}

// ============================================================================
// State Management
// ============================================================================

void GLDevice::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLDevice::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void GLDevice::setDepthTest(bool enabled) {
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void GLDevice::setBlending(bool enabled) {
    if (enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}

void GLDevice::setWireframe(bool enabled) {
    if (enabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

// ============================================================================
// Drawing
// ============================================================================

void GLDevice::draw(VertexArray* vao, uint32_t vertexCount) {
    vao->bind();
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void GLDevice::drawIndexed(VertexArray* vao) {
    vao->bind();
    IndexBuffer* ibo = vao->getIndexBuffer();
    if (ibo) {
        glDrawElements(GL_TRIANGLES, ibo->getCount(), GL_UNSIGNED_INT, nullptr);
    }
}

// ============================================================================
// Factory
// ============================================================================

UNIQUE<GraphicsDevice> GraphicsDevice::create(GraphicsBackend backend) {
    switch (backend) {
        case GraphicsBackend::OpenGL:
            return MAKE_UNIQUE<GLDevice>();

        case GraphicsBackend::Metal:
        case GraphicsBackend::Vulkan:
        case GraphicsBackend::DirectX12:
            std::cerr << "Graphics backend not implemented yet" << std::endl;
            return nullptr;
    }
    return nullptr;
}

} // namespace Pina
