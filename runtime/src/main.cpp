/// Pina Runtime
/// Hot-reload host executable that loads game DLLs
/// TODO: Implement DLL loading and file watching

#include <Pina.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

class RuntimeApp : public Pina::Application {
public:
    RuntimeApp() {
        m_config.title = "Pina Runtime";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vsync = true;
    }

protected:
    void onInit() override {
        std::cout << "Pina Runtime initialized" << std::endl;
        std::cout << "TODO: Implement game DLL loading" << std::endl;
    }

    void onUpdate(float deltaTime) override {
        (void)deltaTime;
        // TODO: Check for DLL changes and hot-reload
    }

    void onRender() override {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: Call game DLL render function
    }
};

PINA_APPLICATION(RuntimeApp)
