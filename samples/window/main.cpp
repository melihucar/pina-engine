/// Window Sample
/// Demonstrates basic window creation with Pina Engine

#include <Pina.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

class WindowSample : public Pina::Application {
public:
    WindowSample() {
        m_config.title = "Pina Engine - Window Sample";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vsync = true;
    }

protected:
    void onInit() override {
        // Called once after initialization
    }

    void onUpdate(float deltaTime) override {
        // Called every frame
        m_time += deltaTime;
    }

    void onRender() override {
        // Clear with a color that cycles based on time
        float r = 0.2f + 0.1f * sinf(m_time);
        float g = 0.3f + 0.1f * sinf(m_time + 2.0f);
        float b = 0.4f + 0.1f * sinf(m_time + 4.0f);

        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void onResize(int width, int height) override {
        glViewport(0, 0, width, height);
    }

    void onShutdown() override {
        // Called before shutdown
    }

private:
    float m_time = 0.0f;
};

PINA_APPLICATION(WindowSample)
