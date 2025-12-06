/// Input Sample
/// Demonstrates keyboard and mouse input with the Input subsystem

#include <Pina.h>
#include <iostream>

class InputSample : public Pina::Application {
public:
    InputSample() {
        m_config.title = "Pina Engine - Input Sample";
        m_config.windowWidth = 1024;
        m_config.windowHeight = 768;
        m_config.vsync = true;
        m_config.resizable = true;
        m_config.clearColor = Pina::Color(0.15f, 0.15f, 0.2f);
    }

protected:
    void onInit() override {
        getDevice()->setDepthTest(true);

        // Setup scene
        m_scene.setDevice(getDevice());

        // Setup simple lighting
        m_scene.setupDefaultLighting();

        // Create floor grid
        for (int x = -2; x <= 2; x++) {
            for (int z = -2; z <= 2; z++) {
                auto* tile = m_scene.createCube("FloorTile", 1.0f);
                tile->setMaterial(Pina::Material::createMatte(Pina::Color(0.3f, 0.3f, 0.4f)));
                tile->getTransform().setLocalPosition(x * 2.0f, -1.0f, z * 2.0f);
                tile->getTransform().setLocalScale(0.9f, 0.1f, 0.9f);
            }
        }

        // Create center cube that responds to input
        m_centerCube = m_scene.createCube("CenterCube", 1.0f);
        m_centerCube->setMaterial(Pina::Material::createPlastic(Pina::Color::white(), 32.0f));

        // Setup camera
        auto* camera = m_scene.getOrCreateDefaultCamera(60.0f);
        camera->lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);

        // Create shader and renderer
        m_shader = getDevice()->createShader();
        m_shader->load(
            Pina::ShaderLibrary::getStandardVertexShader(),
            Pina::ShaderLibrary::getStandardFragmentShader()
        );
        m_renderer = Pina::MAKE_UNIQUE<Pina::SceneRenderer>(getDevice());

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
            m_cubeColor = Pina::Color(1.0f, 0.5f, 0.0f);  // Orange on jump
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
            m_cubeColor = Pina::Color(0.0f, 1.0f, 0.5f);  // Cyan-green
            std::cout << "Left click at: " << input->getMousePosition().x
                      << ", " << input->getMousePosition().y << std::endl;
        }

        // Scroll to zoom
        glm::vec2 scroll = input->getScrollDelta();
        if (scroll.y != 0.0f) {
            m_cameraPos += m_cameraFront * scroll.y * 0.5f;
        }

        // Fade cube color back to white
        m_cubeColor = Pina::Color(
            m_cubeColor.r + (1.0f - m_cubeColor.r) * deltaTime * 2.0f,
            m_cubeColor.g + (1.0f - m_cubeColor.g) * deltaTime * 2.0f,
            m_cubeColor.b + (1.0f - m_cubeColor.b) * deltaTime * 2.0f
        );

        // Update center cube material and rotation
        m_centerCube->setMaterial(Pina::Material::createPlastic(m_cubeColor, 32.0f));
        m_cubeRotation += deltaTime * 30.0f;
        m_centerCube->getTransform().setLocalRotationEuler(0.0f, m_cubeRotation, 0.0f);

        // Update camera
        if (auto* camera = m_scene.getActiveCamera()) {
            camera->lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
        }

        // Update scene
        m_scene.update(deltaTime);
    }

    void onRender() override {
        getDevice()->beginFrame();
        getDevice()->clear(
            m_config.clearColor.r,
            m_config.clearColor.g,
            m_config.clearColor.b
        );

        // Render entire scene
        m_renderer->render(&m_scene, m_shader.get());

        getDevice()->endFrame();
    }

    void onRenderUI() override {
        using namespace Pina::Widgets;
        auto* input = getInput();
        if (!input) return;

        Pina::Color green = Pina::Color::green();
        Pina::Color gray = Pina::Color::gray();

        // Input Debug Window
        setNextWindowSize(Pina::Vector2(280, 0));
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
        getDevice()->setViewport(0, 0, width, height);
        if (auto* camera = m_scene.getActiveCamera()) {
            camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
        }
    }

    void onShutdown() override {
        m_renderer.reset();
        m_shader.reset();
    }

private:
    Pina::Scene m_scene;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::SceneRenderer> m_renderer;

    // Scene nodes
    Pina::Node* m_centerCube = nullptr;

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
    Pina::Color m_cubeColor = Pina::Color::white();
    float m_cubeRotation = 0.0f;
};

PINA_APPLICATION(InputSample)
