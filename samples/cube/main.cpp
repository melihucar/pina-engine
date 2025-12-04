/// Cube Sample
/// Demonstrates 3D rendering with a rotating cube and camera

#include <Pina.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

const char* vertexShaderSource = R"(
#version 410 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 vNormal;
out vec3 vFragPos;
out vec2 vTexCoord;

void main() {
    gl_Position = uMVP * vec4(aPosition, 1.0);
    vFragPos = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 410 core
in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoord;

out vec4 FragColor;

void main() {
    // Simple directional lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(vNormal);

    // Ambient
    vec3 ambient = vec3(0.2);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.8);

    // Color based on position (colorful cube)
    vec3 baseColor = normalize(abs(vNormal)) * 0.5 + 0.5;

    vec3 result = (ambient + diffuse) * baseColor;
    FragColor = vec4(result, 1.0);
}
)";

class CubeSample : public Pina::Application {
public:
    CubeSample() {
        m_config.title = "Pina Engine - Rotating Cube";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
        m_config.vsync = true;
        m_config.resizable = true;
    }

protected:
    void onInit() override {
        // Create graphics device
        m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);
        m_device->setDepthTest(true);

        // Create cube mesh
        m_cube = Pina::CubeMesh::create(m_device.get(), 1.0f);

        // Create and load shader
        m_shader = m_device->createShader();
        if (!m_shader->load(vertexShaderSource, fragmentShaderSource)) {
            std::cerr << "Failed to compile shader!" << std::endl;
        }

        // Set up camera
        m_camera.setPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
        m_camera.lookAt(
            glm::vec3(2.0f, 2.0f, 3.0f),  // Position
            glm::vec3(0.0f, 0.0f, 0.0f),  // Target
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up
        );
    }

    void onUpdate(float deltaTime) override {
        // Rotate cube
        m_rotation += deltaTime * 50.0f;  // 50 degrees per second
        if (m_rotation > 360.0f) {
            m_rotation -= 360.0f;
        }
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.1f, 0.1f, 0.15f);

        // Create model matrix with rotation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(m_rotation * 0.5f), glm::vec3(1.0f, 0.0f, 0.0f));

        // Calculate MVP matrix
        glm::mat4 mvp = m_camera.getViewProjectionMatrix() * model;

        // Set uniforms and draw
        m_shader->bind();
        m_shader->setMat4("uMVP", mvp);
        m_shader->setMat4("uModel", model);
        m_cube->draw(m_shader.get());

        m_device->endFrame();
    }

    void onResize(int width, int height) override {
        m_device->setViewport(0, 0, width, height);
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }

    void onShutdown() override {
        m_shader.reset();
        m_cube.reset();
        m_device.reset();
    }

private:
    Pina::UNIQUE<Pina::GraphicsDevice> m_device;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::CubeMesh> m_cube;
    Pina::Camera m_camera;
    float m_rotation = 0.0f;
};

PINA_APPLICATION(CubeSample)
