#pragma once

#include "Export.h"
#include "Memory.h"
#include <string>

namespace Pina {

class Window;
class GraphicsContext;

/// Application configuration
struct PINA_API ApplicationConfig {
    std::string title = "Pina Application";
    int windowWidth = 1280;
    int windowHeight = 720;
    int minWidth = 0;      // 0 = no minimum
    int minHeight = 0;     // 0 = no minimum
    int maxWidth = 0;      // 0 = no maximum
    int maxHeight = 0;     // 0 = no maximum
    bool vsync = true;
    bool fullscreen = false;
    bool maximized = false;
    bool resizable = false;
};

/// Base application class
/// Games/samples derive from this and override the virtual methods
class PINA_API Application {
public:
    Application();
    virtual ~Application();

    /// Run the application main loop
    /// Returns exit code (uses m_config)
    int run();

    /// Get the application window
    Window* getWindow() const { return m_window.get(); }

    /// Get the graphics context
    GraphicsContext* getContext() const { return m_context.get(); }

protected:
    /// Application configuration - set in subclass constructor
    ApplicationConfig m_config;

    /// Called once after initialization
    virtual void onInit() {}

    /// Called every frame
    virtual void onUpdate(float deltaTime) { (void)deltaTime; }

    /// Called every frame for rendering
    virtual void onRender() {}

    /// Called before shutdown
    virtual void onShutdown() {}

    /// Called when window resizes
    virtual void onResize(int width, int height) { (void)width; (void)height; }

private:
    UNIQUE<Window> m_window;
    UNIQUE<GraphicsContext> m_context;
    bool m_running = false;
};

} // namespace Pina

/// Macro to define the entry point
/// Usage: PINA_APPLICATION(MyGame)
#define PINA_APPLICATION(AppClass) \
    int main(int argc, char** argv) { \
        (void)argc; (void)argv; \
        AppClass app; \
        return app.run(); \
    }
