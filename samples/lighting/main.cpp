/// Lighting Sample
/// Demonstrates directional, point, and spot lights with the lighting system

#include <Pina.h>
#include <iostream>

class LightingSample : public Pina::Application {
public:
    LightingSample() {
        m_config.title = "Pina Engine - Lighting Sample";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vsync = true;
        m_config.resizable = true;
        m_config.clearColor = Pina::Color(0.05f, 0.05f, 0.08f);
    }

protected:
    void onInit() override {
        getDevice()->setDepthTest(true);

        // Setup scene
        m_scene.setDevice(getDevice());

        // Setup lights
        setupLights();

        // Create floor grid using scene primitives
        m_floorMaterial = Pina::Material::createMatte(Pina::Color(0.3f, 0.3f, 0.35f));
        for (int x = -3; x <= 3; x++) {
            for (int z = -3; z <= 3; z++) {
                auto* tile = m_scene.createCube("FloorTile", 1.0f);
                tile->setMaterial(m_floorMaterial);
                tile->getTransform().setLocalPosition(x * 2.0f, -1.0f, z * 2.0f);
                tile->getTransform().setLocalScale(0.95f, 0.1f, 0.95f);
            }
        }

        // Create center rotating cube
        m_centerCube = m_scene.createCube("CenterCube", 1.0f);
        m_centerCube->setMaterial(Pina::Material::createPlastic(Pina::Color::white(), 32.0f));

        // Create light marker cubes
        m_pointLight1Marker = m_scene.createCube("PointLight1Marker", 0.2f);
        m_pointLight2Marker = m_scene.createCube("PointLight2Marker", 0.2f);
        m_spotLightMarker = m_scene.createCube("SpotLightMarker", 0.15f);

        // Setup camera
        auto* camera = m_scene.getOrCreateDefaultCamera(45.0f);
        camera->lookAt(m_cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Create shader and renderer
        m_shader = getDevice()->createShader();
        m_shader->load(
            Pina::ShaderLibrary::getStandardVertexShader(),
            Pina::ShaderLibrary::getStandardFragmentShader()
        );
        m_renderer = Pina::MAKE_UNIQUE<Pina::SceneRenderer>(getDevice());

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
        auto& lightManager = m_scene.getLightManager();

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
        lightManager.addLight(&m_sunLight);
        lightManager.addLight(&m_pointLight1);
        lightManager.addLight(&m_pointLight2);
        lightManager.addLight(&m_spotLight);

        lightManager.setGlobalAmbient(Pina::Color(0.02f, 0.02f, 0.03f));
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
            m_pointLight1Marker->setEnabled(enabled);
            m_pointLight2Marker->setEnabled(enabled);
            std::cout << "Point lights: " << (enabled ? "ON" : "OFF") << std::endl;
        }
        if (input->isKeyPressed(Pina::Key::Num3)) {
            m_spotLight.setEnabled(!m_spotLight.isEnabled());
            m_spotLightMarker->setEnabled(m_spotLight.isEnabled());
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

        // Update camera
        if (auto* camera = m_scene.getActiveCamera()) {
            camera->lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
        }

        // Animate lights
        if (m_animateLights) {
            m_lightTime += deltaTime;

            // Orbit point lights
            float radius = 4.0f;
            Pina::Vector3 pos1(
                cos(m_lightTime) * radius,
                2.0f + sin(m_lightTime * 2.0f) * 0.5f,
                sin(m_lightTime) * radius
            );
            Pina::Vector3 pos2(
                cos(m_lightTime + 3.14159f) * radius,
                2.0f + sin(m_lightTime * 2.0f + 3.14159f) * 0.5f,
                sin(m_lightTime + 3.14159f) * radius
            );
            m_pointLight1.setPosition(pos1);
            m_pointLight2.setPosition(pos2);

            // Update light marker positions
            m_pointLight1Marker->getTransform().setLocalPosition(pos1.x, pos1.y, pos1.z);
            m_pointLight2Marker->getTransform().setLocalPosition(pos2.x, pos2.y, pos2.z);

            // Swing spotlight
            float spotAngle = sin(m_lightTime * 0.5f) * 0.5f;
            m_spotLight.setDirection(Pina::Vector3(spotAngle, -0.7f, -0.7f));
        }

        // Update light marker materials (emissive colors)
        m_pointLight1Marker->setMaterial(Pina::Material::createEmissive(m_pointLight1.getColor(), 1.0f));
        m_pointLight2Marker->setMaterial(Pina::Material::createEmissive(m_pointLight2.getColor(), 1.0f));
        m_spotLightMarker->setMaterial(Pina::Material::createEmissive(Pina::Color::yellow(), 1.0f));

        // Update spotlight marker position
        Pina::Vector3 spotPos = m_spotLight.getPosition();
        m_spotLightMarker->getTransform().setLocalPosition(spotPos.x, spotPos.y, spotPos.z);

        // Rotate center cube
        m_cubeRotation += deltaTime * 30.0f;
        m_centerCube->getTransform().setLocalRotationEuler(0.0f, m_cubeRotation, 0.0f);

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
                snprintf(buf, sizeof(buf), "Active Lights: %d", m_scene.getLightManager().getLightCount());
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
    Pina::Node* m_pointLight1Marker = nullptr;
    Pina::Node* m_pointLight2Marker = nullptr;
    Pina::Node* m_spotLightMarker = nullptr;

    // Lights (owned separately for direct control)
    Pina::DirectionalLight m_sunLight;
    Pina::PointLight m_pointLight1;
    Pina::PointLight m_pointLight2;
    Pina::SpotLight m_spotLight;

    // Materials
    Pina::Material m_floorMaterial;

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
