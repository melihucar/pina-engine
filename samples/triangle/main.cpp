/// Triangle Sample
/// Demonstrates basic rendering with Pina Engine's TriangleMesh

#include <Pina.h>
#include <iostream>

const char* vertexShaderSource = R"(
#version 410 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPosition, 1.0);
    vColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 410 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

class TriangleSample : public Pina::Application {
public:
    TriangleSample() {
        m_config.title = "Pina Engine - Triangle Sample";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
        m_config.vsync = true;
    }

protected:
    void onInit() override {
        // Create graphics device
        m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);

        // Create triangle mesh - handles vertex buffer and layout internally
        m_triangle = Pina::TriangleMesh::create(m_device.get());

        // Create and load shader
        m_shader = m_device->createShader();
        if (!m_shader->load(vertexShaderSource, fragmentShaderSource)) {
            std::cerr << "Failed to compile shader!" << std::endl;
        }
    }

    void onUpdate(float deltaTime) override {
        (void)deltaTime;
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.1f, 0.1f, 0.15f);
        m_triangle->draw(m_shader.get());
        m_device->endFrame();
    }

    void onResize(int width, int height) override {
        m_device->setViewport(0, 0, width, height);
    }

    void onShutdown() override {
        m_shader.reset();
        m_triangle.reset();
        m_device.reset();
    }

private:
    Pina::UNIQUE<Pina::GraphicsDevice> m_device;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::TriangleMesh> m_triangle;
};

PINA_APPLICATION(TriangleSample)
