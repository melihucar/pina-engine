/// Input Sample
/// Demonstrates keyboard and mouse input with the Input subsystem

#include <Pina.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

const char* vertexShaderSource = R"(
#version 410 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 vNormal;
out vec3 vFragPos;

void main() {
    gl_Position = uMVP * vec4(aPosition, 1.0);
    vFragPos = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
}
)";

const char* fragmentShaderSource = R"(
#version 410 core
in vec3 vNormal;
in vec3 vFragPos;

uniform vec3 uColor;

out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(vNormal);

    vec3 ambient = vec3(0.2);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.8);

    vec3 result = (ambient + diffuse) * uColor;
    FragColor = vec4(result, 1.0);
}
)";

class InputSample : public Pina::Application {
public:
    InputSample() {
        m_config.title = "Pina Engine - Input Sample";
        m_config.windowWidth = 1024;
        m_config.windowHeight = 768;
        m_config.vsync = true;
        m_config.resizable = true;
    }

protected:
    void onInit() override {
        m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);
        m_device->setDepthTest(true);

        m_cube = Pina::CubeMesh::create(m_device.get(), 1.0f);

        m_shader = m_device->createShader();
        if (!m_shader->load(vertexShaderSource, fragmentShaderSource)) {
            std::cerr << "Failed to compile shader!" << std::endl;
        }

        m_camera.setPerspective(60.0f, 1024.0f / 768.0f, 0.1f, 100.0f);
        m_camera.lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);

        std::cout << "=== Input Sample ===" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD - Move camera" << std::endl;
        std::cout << "  Right Mouse + Drag - Look around" << std::endl;
        std::cout << "  Scroll - Zoom in/out" << std::endl;
        std::cout << "  Space - Jump (press detection)" << std::endl;
        std::cout << "  Shift - Move faster" << std::endl;
        std::cout << "  Escape - Quit" << std::endl;
        std::cout << "===================" << std::endl;
    }

    void onUpdate(float deltaTime) override {
        auto* input = getInput();
        if (!input) return;

        // Escape to quit
        if (input->isKeyPressed(Pina::Key::Escape)) {
            quit();
            return;
        }

        // Speed modifier with Shift
        float speed = m_moveSpeed;
        if ((input->getModifiers() & Pina::KeyModifier::Shift) != Pina::KeyModifier::None) {
            speed *= 2.5f;
        }

        // WASD movement
        glm::vec3 movement(0.0f);
        if (input->isKeyDown(Pina::Key::W)) {
            movement += m_cameraFront;
        }
        if (input->isKeyDown(Pina::Key::S)) {
            movement -= m_cameraFront;
        }
        if (input->isKeyDown(Pina::Key::A)) {
            movement -= glm::normalize(glm::cross(m_cameraFront, m_cameraUp));
        }
        if (input->isKeyDown(Pina::Key::D)) {
            movement += glm::normalize(glm::cross(m_cameraFront, m_cameraUp));
        }

        if (glm::length(movement) > 0.0f) {
            m_cameraPos += glm::normalize(movement) * speed * deltaTime;
        }

        // Jump detection (press = first frame only)
        if (input->isKeyPressed(Pina::Key::Space)) {
            std::cout << "Jump!" << std::endl;
            m_cubeColor = glm::vec3(1.0f, 0.5f, 0.0f);  // Orange on jump
        }

        // Mouse look with right button
        if (input->isMouseButtonDown(Pina::MouseButton::Right)) {
            glm::vec2 delta = input->getMouseDelta();
            m_yaw += delta.x * m_lookSensitivity;
            m_pitch -= delta.y * m_lookSensitivity;

            // Clamp pitch
            m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

            // Update camera front vector
            glm::vec3 front;
            front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            front.y = sin(glm::radians(m_pitch));
            front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_cameraFront = glm::normalize(front);
        }

        // Left click - change cube color
        if (input->isMouseButtonPressed(Pina::MouseButton::Left)) {
            m_cubeColor = glm::vec3(0.0f, 1.0f, 0.5f);  // Cyan-green
            std::cout << "Left click at: " << input->getMousePosition().x
                      << ", " << input->getMousePosition().y << std::endl;
        }

        // Scroll to zoom
        glm::vec2 scroll = input->getScrollDelta();
        if (scroll.y != 0.0f) {
            m_cameraPos += m_cameraFront * scroll.y * 0.5f;
        }

        // Fade cube color back to white
        m_cubeColor = glm::mix(m_cubeColor, glm::vec3(1.0f), deltaTime * 2.0f);

        // Update camera
        m_camera.lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);

        // Rotate cube slowly
        m_cubeRotation += deltaTime * 30.0f;
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.15f, 0.15f, 0.2f);

        // Draw multiple cubes as a simple scene
        m_shader->bind();

        // Ground plane of cubes
        for (int x = -2; x <= 2; x++) {
            for (int z = -2; z <= 2; z++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x * 2.0f, -1.0f, z * 2.0f));
                model = glm::scale(model, glm::vec3(0.9f, 0.1f, 0.9f));

                glm::mat4 mvp = m_camera.getViewProjectionMatrix() * model;
                m_shader->setMat4("uMVP", mvp);
                m_shader->setMat4("uModel", model);
                m_shader->setVec3("uColor", glm::vec3(0.3f, 0.3f, 0.4f));
                m_cube->draw();
            }
        }

        // Center rotating cube
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(m_cubeRotation), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 mvp = m_camera.getViewProjectionMatrix() * model;
        m_shader->setMat4("uMVP", mvp);
        m_shader->setMat4("uModel", model);
        m_shader->setVec3("uColor", m_cubeColor);
        m_cube->draw();

        m_device->endFrame();
    }

    void onRenderUI() override {
        using namespace Pina::Widgets;
        auto* input = getInput();
        if (!input) return;

        Pina::Color green = Pina::Color::green();
        Pina::Color gray = Pina::Color::gray();
        Pina::Color white = Pina::Color::white();

        // Input Debug Window
        setNextWindowSize(Pina::Vector2(280, 0));  // Wider window, auto height
        Window window("Input Debug", nullptr, Pina::UIWindowFlags::AlwaysAutoResize);
        if (window) {
            // Keyboard section
            if (CollapsingHeader header("Keyboard", Pina::UITreeNodeFlags::DefaultOpen); header) {
                Text("Movement Keys:");
                Text(input->isKeyDown(Pina::Key::W) ? green : gray, "  [W]"); SameLine();
                Text(input->isKeyDown(Pina::Key::A) ? green : gray, "[A]"); SameLine();
                Text(input->isKeyDown(Pina::Key::S) ? green : gray, "[S]"); SameLine();
                Text(input->isKeyDown(Pina::Key::D) ? green : gray, "[D]");

                Separator();
                Text("Modifiers:");
                auto mods = input->getModifiers();
                Text((mods & Pina::KeyModifier::Shift) != Pina::KeyModifier::None ? green : gray, "  [Shift]"); SameLine();
                Text((mods & Pina::KeyModifier::Control) != Pina::KeyModifier::None ? green : gray, "[Ctrl]"); SameLine();
                Text((mods & Pina::KeyModifier::Alt) != Pina::KeyModifier::None ? green : gray, "[Alt]"); SameLine();
                Text((mods & Pina::KeyModifier::Super) != Pina::KeyModifier::None ? green : gray, "[Cmd]");

                Separator();
                Text("Action Keys:");
                Text(input->isKeyDown(Pina::Key::Space) ? green : gray, "  [Space]"); SameLine();
                Text(input->isKeyDown(Pina::Key::Escape) ? green : gray, "[Esc]");
            }

            Separator();

            // Mouse section
            if (CollapsingHeader header("Mouse", Pina::UITreeNodeFlags::DefaultOpen); header) {
                auto pos = input->getMousePosition();
                auto delta = input->getMouseDelta();
                auto scroll = input->getScrollDelta();

                char buf[128];
                snprintf(buf, sizeof(buf), "Position: %.1f, %.1f", pos.x, pos.y);
                Text{buf};

                snprintf(buf, sizeof(buf), "Delta: %.1f, %.1f", delta.x, delta.y);
                Text{delta.x != 0 || delta.y != 0 ? green : gray, buf};

                snprintf(buf, sizeof(buf), "Scroll: %.1f, %.1f", scroll.x, scroll.y);
                Text{scroll.x != 0 || scroll.y != 0 ? green : gray, buf};

                Separator();
                Text("Buttons:");
                Text(input->isMouseButtonDown(Pina::MouseButton::Left) ? green : gray, "  [LMB]"); SameLine();
                Text(input->isMouseButtonDown(Pina::MouseButton::Right) ? green : gray, "[RMB]"); SameLine();
                Text(input->isMouseButtonDown(Pina::MouseButton::Middle) ? green : gray, "[MMB]");
            }

            Separator();

            // Camera info
            if (CollapsingHeader header("Camera", Pina::UITreeNodeFlags::DefaultOpen); header) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Position: %.2f, %.2f, %.2f", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
                Text{buf};
                snprintf(buf, sizeof(buf), "Yaw: %.1f  Pitch: %.1f", m_yaw, m_pitch);
                Text{buf};
            }
        }
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

    // Camera state
    glm::vec3 m_cameraPos{0.0f, 2.0f, 5.0f};
    glm::vec3 m_cameraFront{0.0f, -0.3f, -1.0f};
    glm::vec3 m_cameraUp{0.0f, 1.0f, 0.0f};
    float m_yaw = -90.0f;
    float m_pitch = -15.0f;

    // Movement settings
    float m_moveSpeed = 5.0f;
    float m_lookSensitivity = 0.15f;

    // Cube state
    glm::vec3 m_cubeColor{1.0f, 1.0f, 1.0f};
    float m_cubeRotation = 0.0f;
};

PINA_APPLICATION(InputSample)
