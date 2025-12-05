#include "Application.h"
#include "Context.h"
#include "EventDispatcher.h"
#include "../Platform/Window.h"
#include "../Platform/Graphics.h"
#include "../Input/Input.h"
#include "../UI/UI.h"

// Platform-specific includes for connecting input to window
#ifdef __APPLE__
#include "../Platform/Cocoa/CocoaWindow.h"
#include "../Platform/Cocoa/CocoaInput.h"
#endif

#include <chrono>

namespace Pina {

Application::Application() = default;

Application::~Application() = default;

Window* Application::getWindow() const {
    return m_context ? m_context->getSubsystem<Window>() : nullptr;
}

Graphics* Application::getGraphics() const {
    return m_context ? m_context->getSubsystem<Graphics>() : nullptr;
}

Input* Application::getInput() const {
    return m_context ? m_context->getSubsystem<Input>() : nullptr;
}

UISubsystem* Application::getUI() const {
    return m_context ? m_context->getSubsystem<UISubsystem>() : nullptr;
}

EventDispatcher* Application::getEventDispatcher() const {
    return m_context ? m_context->getSubsystem<EventDispatcher>() : nullptr;
}

void Application::createSubsystems() {
    // Create event dispatcher FIRST (other subsystems may subscribe during init)
    auto* eventDispatcher = new EventDispatcher();
    m_context->registerSubsystem<EventDispatcher>(eventDispatcher);

    // Create window (uses platform default)
    Window* window = Window::createDefault();
    m_context->registerSubsystem<Window>(window);

    // Create graphics (uses platform default with OpenGL backend)
    Graphics* graphics = Graphics::createDefault(GraphicsBackend::OpenGL);
    m_context->registerSubsystem<Graphics>(graphics);

    // Create input (uses platform default)
    Input* input = Input::createDefault(window);
    m_context->registerSubsystem<Input>(input);

    // Create UI (uses platform default)
    UISubsystem* ui = UISubsystem::createDefault();
    m_context->registerSubsystem<UISubsystem>(ui);

    // Platform-specific: connect input handler to window for event routing
#ifdef __APPLE__
    auto* cocoaWindow = dynamic_cast<CocoaWindow*>(window);
    auto* cocoaInput = dynamic_cast<CocoaInput*>(input);
    if (cocoaWindow && cocoaInput) {
        cocoaWindow->setInputHandler(cocoaInput);
        cocoaInput->setEventDispatcher(eventDispatcher);
    }
#endif
}

int Application::run() {
    // Create context
    m_context = MAKE_UNIQUE<Context>();

    // Create subsystems
    createSubsystems();

    // Get subsystems
    auto* window = getWindow();
    auto* graphics = getGraphics();
    auto* input = getInput();
    auto* ui = getUI();

    if (!window || !graphics || !input || !ui) {
        return -1;
    }

    // Configure window
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

    if (!window->create(windowConfig)) {
        return -1;
    }

    // Configure graphics
    GraphicsConfig graphicsConfig;
    graphicsConfig.vsync = m_config.vsync;

    if (!graphics->create(window, graphicsConfig)) {
        return -1;
    }

    // Create UI (requires window and graphics)
    UIConfig uiConfig;
    if (!ui->create(window, graphics, uiConfig)) {
        return -1;
    }

    // Set up window callbacks
    window->setResizeCallback([this, graphics](int w, int h) {
        graphics->updateContext();
        onResize(w, h);
    });

    window->setCloseCallback([this]() {
        m_running = false;
    });

    // Initialize all subsystems
    m_context->initializeSubsystems();

    // User init
    onInit();

    // Main loop
    m_running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running && !window->shouldClose()) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Poll events
        window->pollEvents();

        // Update subsystems
        m_context->updateSubsystems(deltaTime);

        // Update
        onUpdate(deltaTime);

        // Render
        graphics->makeCurrent();
        onRender();

        // UI rendering
        ui->beginFrame();
        onRenderUI();
        ui->endFrame();

        graphics->swapBuffers();

        // End frame for input (clear per-frame state)
        input->endFrame();
    }

    // User shutdown
    onShutdown();

    // Shutdown subsystems (in reverse order)
    m_context->shutdownSubsystems();

    return 0;
}

} // namespace Pina
