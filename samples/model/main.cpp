/// Model Sample
/// Demonstrates loading and rendering 3D models

#include <Pina.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cstdio>

class ModelSample : public Pina::Application {
public:
    ModelSample() {
        m_config.title = "Pina Engine - Model Sample";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vsync = true;
        m_config.resizable = true;
    }

protected:
    void onInit() override {
        m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);
        m_device->setDepthTest(true);

        // Create shader
        m_shader = m_device->createShader();
        if (!m_shader->load(Pina::ShaderLibrary::getStandardVertexShader(),
                            Pina::ShaderLibrary::getStandardFragmentShader())) {
            std::cerr << "Failed to compile shader!" << std::endl;
        }

        // Load 3D model
        m_model = Pina::Model::load(m_device.get(), "assets/scene.gltf");
        if (!m_model) {
            std::cerr << "Failed to load model!" << std::endl;
            // Create a fallback cube
            m_fallbackCube = Pina::CubeMesh::create(m_device.get(), 1.0f);
        } else {
            // Calculate auto-fit scale and center offset
            m_modelScale = m_model->calculateFitScale(2.0f);  // Fit within 2 units
            m_modelCenter = m_model->getCenter();
            std::cout << "Model bounds: " << m_model->getSize().x << " x "
                      << m_model->getSize().y << " x " << m_model->getSize().z << std::endl;
            std::cout << "Auto-fit scale: " << m_modelScale << std::endl;
        }

        // Create floor
        m_floor = Pina::QuadMesh::create(m_device.get(), 10.0f, 10.0f);
        m_floorMaterial = Pina::Material::createMatte(Pina::Color(0.3f, 0.3f, 0.35f));

        // Setup camera
        m_camera.setPerspective(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_camera.lookAt(m_cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Setup lights
        setupLights();

        std::cout << "=== Model Sample ===" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD - Move camera" << std::endl;
        std::cout << "  Right Mouse + Drag - Look around" << std::endl;
        std::cout << "  Space - Toggle rotation" << std::endl;
        std::cout << "  Escape - Quit" << std::endl;
        std::cout << "====================" << std::endl;
    }

    void setupLights() {
        // Main directional light
        m_sunLight.setDirection(Pina::Vector3(-0.5f, -1.0f, -0.3f));
        m_sunLight.setColor(Pina::Color(1.0f, 0.95f, 0.9f));
        m_sunLight.setIntensity(0.8f);
        m_sunLight.setAmbient(Pina::Color(0.15f, 0.15f, 0.18f));

        // Fill light
        m_pointLight.setPosition(Pina::Vector3(3.0f, 2.0f, 3.0f));
        m_pointLight.setColor(Pina::Color(0.8f, 0.8f, 1.0f));
        m_pointLight.setIntensity(0.5f);
        m_pointLight.setRange(15.0f);

        m_lightManager.addLight(&m_sunLight);
        m_lightManager.addLight(&m_pointLight);
        m_lightManager.setGlobalAmbient(Pina::Color(0.05f, 0.05f, 0.07f));
    }

    void onUpdate(float deltaTime) override {
        auto* input = getInput();
        if (!input) return;

        if (input->isKeyPressed(Pina::Key::Escape)) {
            quit();
            return;
        }

        // Toggle rotation
        if (input->isKeyPressed(Pina::Key::Space)) {
            m_rotate = !m_rotate;
            std::cout << "Rotation: " << (m_rotate ? "ON" : "OFF") << std::endl;
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

        // Update rotation
        if (m_rotate) {
            m_rotation += deltaTime * 30.0f;
        }

        m_lightManager.update();
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.1f, 0.1f, 0.12f);

        m_shader->bind();

        // Set camera matrices
        m_shader->setMat4("uView", m_camera.getViewMatrix());
        m_shader->setMat4("uProjection", m_camera.getProjectionMatrix());

        // Upload lighting
        m_lightManager.setViewPosition(m_cameraPos);
        m_lightManager.uploadToShader(m_shader.get());

        // Draw model
        if (m_model) {
            glm::mat4 model = glm::mat4(1.0f);
            // Rotate animation around Y axis
            model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            // Scale to fit
            model = glm::scale(model, glm::vec3(m_modelScale));
            // Stand the model up (glTF may have Z-up, we need Y-up)
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            // Center the model (translate by negative center to bring center to origin)
            model = glm::translate(model, -m_modelCenter);

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", normalMatrix);

            m_model->draw(m_shader.get(), &m_lightManager);
        } else if (m_fallbackCube) {
            // Draw fallback cube
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
            model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", normalMatrix);

            auto fallbackMat = Pina::Material::createPlastic(Pina::Color::red(), 32.0f);
            m_lightManager.uploadMaterial(m_shader.get(), fallbackMat);
            m_fallbackCube->draw();
        }

        // Draw floor
        m_lightManager.uploadMaterial(m_shader.get(), m_floorMaterial);
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", normalMatrix);
            m_floor->draw();
        }

        m_device->endFrame();
    }

    void onRenderUI() override {
        using namespace Pina::Widgets;

        Pina::Color green = Pina::Color::green();
        Pina::Color gray = Pina::Color::gray();

        setNextWindowSize(Pina::Vector2(220, 0));
        Window window("Model Loader", nullptr, Pina::UIWindowFlags::AlwaysAutoResize);
        if (window) {
            if (CollapsingHeader header("Model Info", Pina::UITreeNodeFlags::DefaultOpen); header) {
                if (m_model) {
                    Text{green, "Model loaded successfully"};
                    char meshBuf[32], matBuf[32];
                    snprintf(meshBuf, sizeof(meshBuf), "Meshes: %zu", m_model->getMeshCount());
                    snprintf(matBuf, sizeof(matBuf), "Materials: %zu", m_model->getMaterialCount());
                    { Text t1(meshBuf); }
                    { Text t2(matBuf); }
                } else {
                    Text{Pina::Color::red(), "Model failed to load"};
                    Text{"Using fallback cube"};
                }
            }

            Separator();

            if (CollapsingHeader header("Animation", Pina::UITreeNodeFlags::DefaultOpen); header) {
                Text(m_rotate ? green : gray, "[Space] Rotation");
            }

            Separator();

            if (CollapsingHeader header("Camera", Pina::UITreeNodeFlags::DefaultOpen); header) {
                char posBuf[64], angleBuf[48];
                snprintf(posBuf, sizeof(posBuf), "Pos: (%.1f, %.1f, %.1f)", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
                snprintf(angleBuf, sizeof(angleBuf), "Yaw: %.1f  Pitch: %.1f", m_yaw, m_pitch);
                { Text t1(posBuf); }
                { Text t2(angleBuf); }
            }
        }
    }

    void onResize(int width, int height) override {
        m_device->setViewport(0, 0, width, height);
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }

    void onShutdown() override {
        m_shader.reset();
        m_model.reset();
        m_fallbackCube.reset();
        m_floor.reset();
        m_device.reset();
    }

private:
    Pina::UNIQUE<Pina::GraphicsDevice> m_device;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::Model> m_model;
    Pina::UNIQUE<Pina::CubeMesh> m_fallbackCube;
    Pina::UNIQUE<Pina::QuadMesh> m_floor;
    Pina::Camera m_camera;

    // Materials
    Pina::Material m_floorMaterial;

    // Lights
    Pina::LightManager m_lightManager;
    Pina::DirectionalLight m_sunLight;
    Pina::PointLight m_pointLight;

    // Camera state
    glm::vec3 m_cameraPos{0.0f, 2.0f, 5.0f};
    glm::vec3 m_cameraFront{0.0f, -0.3f, -1.0f};
    glm::vec3 m_cameraUp{0.0f, 1.0f, 0.0f};
    float m_yaw = -90.0f;
    float m_pitch = -15.0f;

    // Animation
    float m_rotation = 0.0f;
    bool m_rotate = true;

    // Model auto-fit
    float m_modelScale = 1.0f;
    glm::vec3 m_modelCenter{0.0f};
};

PINA_APPLICATION(ModelSample)
