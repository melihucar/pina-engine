#pragma once

/// Pina Engine - Application Base Class
/// Games and samples derive from this class

#include "Export.h"
#include "Memory.h"
#include "Context.h"
#include <string>

namespace Pina {

class Window;
class Graphics;
class Input;
class UISubsystem;

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
    int run();

    /// Request application to quit
    void quit() { m_running = false; }

    // ========================================================================
    // Subsystem Access
    // ========================================================================

    /// Get the engine context (subsystem registry)
    Context* getContext() { return m_context.get(); }

    /// Get a subsystem by type
    template<typename T>
    T* getSubsystem() const {
        return m_context ? m_context->getSubsystem<T>() : nullptr;
    }

    /// Convenience accessors for common subsystems
    Window* getWindow() const;
    Graphics* getGraphics() const;
    Input* getInput() const;
    UISubsystem* getUI() const;

protected:
    /// Application configuration - set in subclass constructor
    ApplicationConfig m_config;

    /// Called once after initialization
    virtual void onInit() {}

    /// Called every frame
    virtual void onUpdate(float deltaTime) { (void)deltaTime; }

    /// Called every frame for rendering
    virtual void onRender() {}

    /// Called every frame for UI rendering (after onRender, before swap)
    virtual void onRenderUI() {}

    /// Called before shutdown
    virtual void onShutdown() {}

    /// Called when window resizes
    virtual void onResize(int width, int height) { (void)width; (void)height; }

private:
    void createSubsystems();

    UNIQUE<Context> m_context;
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
