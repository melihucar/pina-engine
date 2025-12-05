/// Lighting Sample
/// Demonstrates directional, point, and spot lights with the lighting system

#include <Pina.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class LightingSample : public Pina::Application {
public:
    LightingSample() {
        m_config.title = "Pina Engine - Lighting Sample";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vsync = true;
        m_config.resizable = true;
    }

protected:
    void onInit() override {
        m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);
        m_device->setDepthTest(true);

        // Create meshes
        m_cube = Pina::CubeMesh::create(m_device.get(), 1.0f);

        // Load standard lit shader
        m_shader = m_device->createShader();
        if (!m_shader->load(Pina::ShaderLibrary::getStandardVertexShader(),
                            Pina::ShaderLibrary::getStandardFragmentShader())) {
            std::cerr << "Failed to compile lighting shader!" << std::endl;
        }

        // Setup camera
        m_camera.setPerspective(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_camera.lookAt(m_cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Setup lights
        setupLights();

        // Setup materials
        m_floorMaterial = Pina::Material::createMatte(Pina::Color(0.3f, 0.3f, 0.35f));
        m_cubeMaterial = Pina::Material::createPlastic(Pina::Color::white(), 32.0f);

        std::cout << "=== Lighting Sample ===" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD - Move camera" << std::endl;
        std::cout << "  Right Mouse + Drag - Look around" << std::endl;
        std::cout << "  1/2/3 - Toggle Sun/Point/Spot lights" << std::endl;
        std::cout << "  Space - Animate lights" << std::endl;
        std::cout << "  Escape - Quit" << std::endl;
        std::cout << "=======================" << std::endl;
    }

    void setupLights() {
        // Sun light (directional)
        m_sunLight.setDirection(Pina::Vector3(-0.5f, -1.0f, -0.3f));
        m_sunLight.setColor(Pina::Color(1.0f, 0.95f, 0.8f));
        m_sunLight.setIntensity(0.6f);
        m_sunLight.setAmbient(Pina::Color(0.05f, 0.05f, 0.08f));

        // Red point light
        m_pointLight1.setPosition(Pina::Vector3(3.0f, 2.0f, 0.0f));
        m_pointLight1.setColor(Pina::Color::red());
        m_pointLight1.setIntensity(1.5f);
        m_pointLight1.setRange(15.0f);

        // Blue point light
        m_pointLight2.setPosition(Pina::Vector3(-3.0f, 2.0f, 0.0f));
        m_pointLight2.setColor(Pina::Color::blue());
        m_pointLight2.setIntensity(1.5f);
        m_pointLight2.setRange(15.0f);

        // Spotlight (flashlight effect)
        m_spotLight.setPosition(Pina::Vector3(0.0f, 5.0f, 5.0f));
        m_spotLight.setDirection(Pina::Vector3(0.0f, -0.7f, -0.7f));
        m_spotLight.setColor(Pina::Color::white());
        m_spotLight.setIntensity(2.0f);
        m_spotLight.setInnerCutoff(10.0f);
        m_spotLight.setOuterCutoff(15.0f);
        m_spotLight.setRange(20.0f);

        // Add lights to manager
        m_lightManager.addLight(&m_sunLight);
        m_lightManager.addLight(&m_pointLight1);
        m_lightManager.addLight(&m_pointLight2);
        m_lightManager.addLight(&m_spotLight);

        m_lightManager.setGlobalAmbient(Pina::Color(0.02f, 0.02f, 0.03f));
    }

    void onUpdate(float deltaTime) override {
        auto* input = getInput();
        if (!input) return;

        // Escape to quit
        if (input->isKeyPressed(Pina::Key::Escape)) {
            quit();
            return;
        }

        // Toggle lights
        if (input->isKeyPressed(Pina::Key::Num1)) {
            m_sunLight.setEnabled(!m_sunLight.isEnabled());
            std::cout << "Sun light: " << (m_sunLight.isEnabled() ? "ON" : "OFF") << std::endl;
        }
        if (input->isKeyPressed(Pina::Key::Num2)) {
            bool enabled = !m_pointLight1.isEnabled();
            m_pointLight1.setEnabled(enabled);
            m_pointLight2.setEnabled(enabled);
            std::cout << "Point lights: " << (enabled ? "ON" : "OFF") << std::endl;
        }
        if (input->isKeyPressed(Pina::Key::Num3)) {
            m_spotLight.setEnabled(!m_spotLight.isEnabled());
            std::cout << "Spot light: " << (m_spotLight.isEnabled() ? "ON" : "OFF") << std::endl;
        }

        // Toggle animation
        if (input->isKeyPressed(Pina::Key::Space)) {
            m_animateLights = !m_animateLights;
            std::cout << "Light animation: " << (m_animateLights ? "ON" : "OFF") << std::endl;
        }

        // Camera movement
        float speed = 5.0f * deltaTime;
        if (input->isKeyDown(Pina::Key::W)) m_cameraPos += m_cameraFront * speed;
        if (input->isKeyDown(Pina::Key::S)) m_cameraPos -= m_cameraFront * speed;
        if (input->isKeyDown(Pina::Key::A)) m_cameraPos -= glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * speed;
        if (input->isKeyDown(Pina::Key::D)) m_cameraPos += glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * speed;

        // Mouse look
        if (input->isMouseButtonDown(Pina::MouseButton::Right)) {
            glm::vec2 delta = input->getMouseDelta();
            m_yaw += delta.x * 0.15f;
            m_pitch -= delta.y * 0.15f;
            m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

            glm::vec3 front;
            front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            front.y = sin(glm::radians(m_pitch));
            front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_cameraFront = glm::normalize(front);
        }

        m_camera.lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);

        // Animate lights
        if (m_animateLights) {
            m_lightTime += deltaTime;

            // Orbit point lights
            float radius = 4.0f;
            m_pointLight1.setPosition(Pina::Vector3(
                cos(m_lightTime) * radius,
                2.0f + sin(m_lightTime * 2.0f) * 0.5f,
                sin(m_lightTime) * radius
            ));
            m_pointLight2.setPosition(Pina::Vector3(
                cos(m_lightTime + 3.14159f) * radius,
                2.0f + sin(m_lightTime * 2.0f + 3.14159f) * 0.5f,
                sin(m_lightTime + 3.14159f) * radius
            ));

            // Swing spotlight
            float spotAngle = sin(m_lightTime * 0.5f) * 0.5f;
            m_spotLight.setDirection(Pina::Vector3(spotAngle, -0.7f, -0.7f));
        }

        // Update light data
        m_lightManager.update();

        // Rotate center cube
        m_cubeRotation += deltaTime * 30.0f;
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.05f, 0.05f, 0.08f);

        m_shader->bind();

        // Set view/projection matrices
        m_shader->setMat4("uView", m_camera.getViewMatrix());
        m_shader->setMat4("uProjection", m_camera.getProjectionMatrix());

        // Upload lighting
        m_lightManager.setViewPosition(m_cameraPos);
        m_lightManager.uploadToShader(m_shader.get());

        // Draw floor (grid of cubes)
        m_lightManager.uploadMaterial(m_shader.get(), m_floorMaterial);
        for (int x = -3; x <= 3; x++) {
            for (int z = -3; z <= 3; z++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x * 2.0f, -1.0f, z * 2.0f));
                model = glm::scale(model, glm::vec3(0.95f, 0.1f, 0.95f));

                glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
                m_shader->setMat4("uModel", model);
                m_shader->setMat3("uNormalMatrix", normalMatrix);
                m_cube->draw();
            }
        }

        // Draw center rotating cube
        m_lightManager.uploadMaterial(m_shader.get(), m_cubeMaterial);
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::rotate(model, glm::radians(m_cubeRotation), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", normalMatrix);
            m_cube->draw();
        }

        // Draw small cubes at light positions (for visualization)
        Pina::Material lightMarker = Pina::Material::createEmissive(Pina::Color::white(), 1.0f);

        // Point light 1 marker
        if (m_pointLight1.isEnabled()) {
            Pina::Material marker = Pina::Material::createEmissive(m_pointLight1.getColor(), 1.0f);
            m_lightManager.uploadMaterial(m_shader.get(), marker);
            Pina::Vector3 pos = m_pointLight1.getPosition();
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
            model = glm::scale(model, glm::vec3(0.2f));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", glm::mat3(1.0f));
            m_cube->draw();
        }

        // Point light 2 marker
        if (m_pointLight2.isEnabled()) {
            Pina::Material marker = Pina::Material::createEmissive(m_pointLight2.getColor(), 1.0f);
            m_lightManager.uploadMaterial(m_shader.get(), marker);
            Pina::Vector3 pos = m_pointLight2.getPosition();
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
            model = glm::scale(model, glm::vec3(0.2f));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", glm::mat3(1.0f));
            m_cube->draw();
        }

        // Spotlight marker
        if (m_spotLight.isEnabled()) {
            Pina::Material marker = Pina::Material::createEmissive(Pina::Color::yellow(), 1.0f);
            m_lightManager.uploadMaterial(m_shader.get(), marker);
            Pina::Vector3 pos = m_spotLight.getPosition();
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
            model = glm::scale(model, glm::vec3(0.15f));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", glm::mat3(1.0f));
            m_cube->draw();
        }

        m_device->endFrame();
    }

    void onRenderUI() override {
        using namespace Pina::Widgets;

        Pina::Color green = Pina::Color::green();
        Pina::Color gray = Pina::Color::gray();

        setNextWindowSize(Pina::Vector2(250, 0));
        Window window("Lighting", nullptr, Pina::UIWindowFlags::AlwaysAutoResize);
        if (window) {
            if (CollapsingHeader header("Lights", Pina::UITreeNodeFlags::DefaultOpen); header) {
                Text(m_sunLight.isEnabled() ? green : gray, "[1] Sun Light");
                Text(m_pointLight1.isEnabled() ? green : gray, "[2] Point Lights (Red/Blue)");
                Text(m_spotLight.isEnabled() ? green : gray, "[3] Spot Light");
                Separator();
                Text(m_animateLights ? green : gray, "[Space] Animation");
            }

            Separator();

            if (CollapsingHeader header("Camera", Pina::UITreeNodeFlags::DefaultOpen); header) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Pos: %.1f, %.1f, %.1f",
                         m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
                Text{buf};
            }

            Separator();

            if (CollapsingHeader header("Info", Pina::UITreeNodeFlags::DefaultOpen); header) {
                char buf[64];
                snprintf(buf, sizeof(buf), "Active Lights: %d", m_lightManager.getLightCount());
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

    // Lights
    Pina::LightManager m_lightManager;
    Pina::DirectionalLight m_sunLight;
    Pina::PointLight m_pointLight1;
    Pina::PointLight m_pointLight2;
    Pina::SpotLight m_spotLight;

    // Materials
    Pina::Material m_floorMaterial;
    Pina::Material m_cubeMaterial;

    // Camera state
    glm::vec3 m_cameraPos{0.0f, 3.0f, 8.0f};
    glm::vec3 m_cameraFront{0.0f, -0.3f, -1.0f};
    glm::vec3 m_cameraUp{0.0f, 1.0f, 0.0f};
    float m_yaw = -90.0f;
    float m_pitch = -15.0f;

    // Animation
    float m_cubeRotation = 0.0f;
    float m_lightTime = 0.0f;
    bool m_animateLights = true;
};

PINA_APPLICATION(LightingSample)
