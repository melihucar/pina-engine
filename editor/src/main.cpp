/// Pina Editor
/// Dear ImGui based editor application
/// TODO: Implement Dear ImGui integration and editor panels

#include <Pina.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

class EditorApp : public Pina::Application {
public:
    EditorApp() {
        m_config.title = "Pina Editor";
        m_config.windowWidth = 1600;
        m_config.windowHeight = 900;
        m_config.vsync = true;
    }

protected:
    void onInit() override {
        std::cout << "Pina Editor initialized" << std::endl;
        std::cout << "TODO: Initialize Dear ImGui" << std::endl;
    }

    void onUpdate(float deltaTime) override {
        (void)deltaTime;
        // TODO: Update ImGui
    }

    void onRender() override {
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: Render ImGui
    }
};

PINA_APPLICATION(EditorApp)
