#include "Application.h"
#include "../Platform/Window.h"
#include "../Platform/GraphicsContext.h"

#include <chrono>

namespace Pina {

Application::Application() = default;

Application::~Application() = default;

int Application::run() {
    // Create window
    m_window.reset(Window::create());
    if (!m_window) {
        return -1;
    }

    WindowConfig windowConfig;
    windowConfig.width = m_config.windowWidth;
    windowConfig.height = m_config.windowHeight;
    windowConfig.minWidth = m_config.minWidth;
    windowConfig.minHeight = m_config.minHeight;
    windowConfig.maxWidth = m_config.maxWidth;
    windowConfig.maxHeight = m_config.maxHeight;
    windowConfig.title = m_config.title;
    windowConfig.fullscreen = m_config.fullscreen;
    windowConfig.maximized = m_config.maximized;
    windowConfig.resizable = m_config.resizable;

    if (!m_window->create(windowConfig)) {
        return -1;
    }

    // Create graphics context
    m_context.reset(GraphicsContext::create(GraphicsBackend::OpenGL));
    if (!m_context) {
        return -1;
    }

    ContextConfig contextConfig;
    contextConfig.vsync = m_config.vsync;

    if (!m_context->create(m_window.get(), contextConfig)) {
        return -1;
    }

    // Set up window callbacks (after context is created)
    m_window->setResizeCallback([this](int w, int h) {
        m_context->update();
        onResize(w, h);
    });

    m_window->setCloseCallback([this]() {
        m_running = false;
    });

    // Initialize
    onInit();

    // Main loop
    m_running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running && !m_window->shouldClose()) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Poll events
        m_window->pollEvents();

        // Update
        onUpdate(deltaTime);

        // Render
        m_context->makeCurrent();
        onRender();
        m_context->swapBuffers();
    }

    // Shutdown
    onShutdown();

    // Cleanup
    m_context->destroy();
    m_window->destroy();

    return 0;
}

} // namespace Pina
