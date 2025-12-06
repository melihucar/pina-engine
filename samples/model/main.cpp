/// Model Sample
/// Demonstrates loading and rendering 3D models with PBR support

#include <Pina.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cstdio>

enum class ModelType {
    Gameboy,
    PostApocalyptic,
    Vehicle
};

enum class CameraMode {
    Orbit,
    Freelook
};

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

        // Create shaders (standard Blinn-Phong and PBR)
        m_shader = m_device->createShader();
        if (!m_shader->load(Pina::ShaderLibrary::getStandardVertexShader(),
                            Pina::ShaderLibrary::getStandardFragmentShader())) {
            std::cerr << "Failed to compile standard shader!" << std::endl;
        }

        m_pbrShader = m_device->createShader();
        if (!m_pbrShader->load(Pina::ShaderLibrary::getPBRVertexShader(),
                               Pina::ShaderLibrary::getPBRFragmentShader())) {
            std::cerr << "Failed to compile PBR shader!" << std::endl;
        }

        // Load initial model
        loadModel(m_selectedModel);

        // Create light cube for visualization
        m_lightCube = Pina::CubeMesh::create(m_device.get(), 0.15f);

        // Setup camera with initial position
        m_camera.setPerspective(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_camera.lookAt(m_cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Setup camera controllers
        m_orbitCamera = std::make_unique<Pina::OrbitCamera>(&m_camera);
        m_orbitCamera->setTarget(glm::vec3(0.0f, 1.0f, 0.0f));
        m_orbitCamera->setDistance(5.0f);
        m_orbitCamera->setRotation(-30.0f, 20.0f);

        m_freelookCamera = std::make_unique<Pina::FreelookCamera>(&m_camera);
        m_freelookCamera->setMoveSpeed(5.0f);

        // Setup lights
        setupLights();

        std::cout << "=== Model Sample ===" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  Orbit Mode: Right Mouse + Drag - Rotate, Middle Mouse - Pan, Scroll - Zoom" << std::endl;
        std::cout << "  Freelook Mode: WASD - Move, Right Mouse + Drag - Look around" << std::endl;
        std::cout << "  Space - Toggle rotation" << std::endl;
        std::cout << "  Z - Toggle wireframe" << std::endl;
        std::cout << "  Escape - Quit" << std::endl;
        std::cout << "====================" << std::endl;
    }

    void loadModel(ModelType type) {
        const char* path = nullptr;
        switch (type) {
            case ModelType::Gameboy:
                path = "assets/gameboy/scene.gltf";
                break;
            case ModelType::PostApocalyptic:
                path = "assets/post_apocalyptic/scene.gltf";
                break;
            case ModelType::Vehicle:
                path = "assets/vehicle/scene.gltf";
                break;
        }

        m_model = Pina::Model::load(m_device.get(), path);
        if (!m_model) {
            std::cerr << "Failed to load model: " << path << std::endl;
            // Create a fallback cube
            m_fallbackCube = Pina::CubeMesh::create(m_device.get(), 1.0f);
            m_modelScale = 1.0f;
            m_modelCenter = glm::vec3(0.0f);
            m_modelBaseY = 0.5f;
            m_usePBR = false;
        } else {
            m_fallbackCube.reset();
            // Calculate auto-fit scale and center offset
            m_modelScale = m_model->calculateFitScale(2.0f);
            m_modelCenter = m_model->getCenter();
            m_modelBaseY = -m_model->getBoundingBox().min.y * m_modelScale;
            m_usePBR = m_model->hasPBRMaterials();

            std::cout << "Loaded model: " << path << std::endl;
            std::cout << "  Meshes: " << m_model->getMeshCount() << std::endl;
            std::cout << "  Materials: " << m_model->getMaterialCount() << std::endl;
            std::cout << "  PBR Materials: " << (m_usePBR ? "Yes" : "No") << std::endl;
            std::cout << "  Size: " << m_model->getSize().x << " x "
                      << m_model->getSize().y << " x " << m_model->getSize().z << std::endl;
            std::cout << "  Auto-fit scale: " << m_modelScale << std::endl;
        }

        // Reset transform
        m_modelPosition = glm::vec3(0.0f);
        m_modelRotation = glm::vec3(0.0f);
        m_autoRotation = 0.0f;

        // Reset camera to best view of the model
        if (m_model && m_orbitCamera) {
            glm::vec3 center = m_model->getCenter() * m_modelScale;
            center.y += m_modelBaseY;
            float size = m_model->getBoundingBox().getMaxDimension() * m_modelScale;
            m_orbitCamera->focusOn(center, size);
        }
    }

    void setupLights() {
        // Main directional light (sun) - key light from above-left
        m_sunLight.setDirection(Pina::Vector3(-0.5f, -1.0f, -0.3f));
        m_sunLight.setColor(Pina::Color(1.0f, 0.98f, 0.95f));
        m_sunLight.setIntensity(1.2f);
        m_sunLight.setAmbient(Pina::Color(0.2f, 0.2f, 0.22f));

        // Front fill light - illuminates the front of the model
        m_pointLight.setPosition(Pina::Vector3(3.0f, 3.0f, 5.0f));
        m_pointLight.setColor(Pina::Color(1.0f, 0.95f, 0.9f));
        m_pointLight.setIntensity(0.8f);
        m_pointLight.setRange(20.0f);

        // Back fill light - prevents dark shadows on the back
        m_backLight.setPosition(Pina::Vector3(-3.0f, 2.0f, -4.0f));
        m_backLight.setColor(Pina::Color(0.7f, 0.8f, 1.0f));
        m_backLight.setIntensity(0.5f);
        m_backLight.setRange(20.0f);

        m_lightManager.addLight(&m_sunLight);
        m_lightManager.addLight(&m_pointLight);
        m_lightManager.addLight(&m_backLight);
        m_lightManager.setGlobalAmbient(Pina::Color(0.15f, 0.15f, 0.18f));
    }

    void onUpdate(float deltaTime) override {
        auto* input = getInput();
        if (!input) return;

        if (input->isKeyPressed(Pina::Key::Escape)) {
            quit();
            return;
        }

        // Toggle auto-rotation
        if (input->isKeyPressed(Pina::Key::Space)) {
            m_autoRotate = !m_autoRotate;
            std::cout << "Auto-rotation: " << (m_autoRotate ? "ON" : "OFF") << std::endl;
        }

        // Toggle wireframe mode
        if (input->isKeyPressed(Pina::Key::Z)) {
            m_wireframe = !m_wireframe;
            std::cout << "Wireframe: " << (m_wireframe ? "ON" : "OFF") << std::endl;
        }

        // Update camera based on selected mode
        switch (m_cameraMode) {
            case CameraMode::Orbit:
                m_orbitCamera->update(input, deltaTime);
                break;
            case CameraMode::Freelook:
                m_freelookCamera->update(input, deltaTime);
                break;
        }

        // Update auto-rotation (Y axis only)
        if (m_autoRotate) {
            m_autoRotation += deltaTime * 30.0f;
        }

        m_lightManager.update();
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.1f, 0.1f, 0.12f);

        // Select shader based on material type
        Pina::Shader* activeShader = m_usePBR ? m_pbrShader.get() : m_shader.get();
        activeShader->bind();

        // Set camera matrices
        activeShader->setMat4("uView", m_camera.getViewMatrix());
        activeShader->setMat4("uProjection", m_camera.getProjectionMatrix());

        // Upload lighting
        m_lightManager.setViewPosition(m_camera.getPosition());
        m_lightManager.uploadToShader(activeShader);

        // Apply wireframe mode if enabled
        m_device->setWireframe(m_wireframe);
        if (m_usePBR) {
            activeShader->setInt("uShadingMode", m_wireframe ? 2 : 0);
        } else {
            activeShader->setInt("uWireframe", m_wireframe ? 1 : 0);
        }

        // Enable blending for alpha transparency
        m_device->setBlending(true);

        // Draw model
        if (m_model) {
            glm::mat4 model = glm::mat4(1.0f);
            // Position offset (including floor placement)
            model = glm::translate(model, m_modelPosition + glm::vec3(0.0f, m_modelBaseY, 0.0f));
            // Auto-rotation around Y axis
            model = glm::rotate(model, glm::radians(m_autoRotation), glm::vec3(0.0f, 1.0f, 0.0f));
            // Manual rotation controls (applied in Y, X, Z order)
            model = glm::rotate(model, glm::radians(m_modelRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(m_modelRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(m_modelRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            // Scale to fit
            model = glm::scale(model, glm::vec3(m_modelScale));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            activeShader->setMat4("uModel", model);
            activeShader->setMat3("uNormalMatrix", normalMatrix);

            m_model->draw(activeShader, &m_lightManager);
        } else if (m_fallbackCube) {
            // Draw fallback cube
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
            model = glm::rotate(model, glm::radians(m_autoRotation), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            activeShader->setMat4("uModel", model);
            activeShader->setMat3("uNormalMatrix", normalMatrix);

            auto fallbackMat = Pina::Material::createPlastic(Pina::Color::red(), 32.0f);
            m_lightManager.uploadMaterial(activeShader, fallbackMat);
            m_fallbackCube->draw();
        }

        // Disable blending for subsequent rendering
        m_device->setBlending(false);

        // Draw light cubes (point lights only)
        if (m_lightCube) {
            m_shader->bind();
            m_shader->setMat4("uView", m_camera.getViewMatrix());
            m_shader->setMat4("uProjection", m_camera.getProjectionMatrix());
            m_shader->setInt("uWireframe", 0);
            m_lightManager.uploadToShader(m_shader.get());

            // Draw front fill light cube
            {
                Pina::Vector3 pos = m_pointLight.getPosition();
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
                glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
                m_shader->setMat4("uModel", model);
                m_shader->setMat3("uNormalMatrix", normalMatrix);

                Pina::Color lightColor = m_pointLight.getColor();
                auto lightMat = Pina::Material::createMatte(Pina::Color::white());
                lightMat.setEmissive(lightColor);
                m_lightManager.uploadMaterial(m_shader.get(), lightMat);
                m_lightCube->draw();
            }

            // Draw back fill light cube
            {
                Pina::Vector3 pos = m_backLight.getPosition();
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
                glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
                m_shader->setMat4("uModel", model);
                m_shader->setMat3("uNormalMatrix", normalMatrix);

                Pina::Color lightColor = m_backLight.getColor();
                auto lightMat = Pina::Material::createMatte(Pina::Color::white());
                lightMat.setEmissive(lightColor);
                m_lightManager.uploadMaterial(m_shader.get(), lightMat);
                m_lightCube->draw();
            }
        }

        // Reset to solid mode for UI rendering
        m_device->setWireframe(false);

        m_device->endFrame();
    }

    void onRenderUI() override {
        using namespace Pina::Widgets;

        Pina::Color green = Pina::Color::green();

        setNextWindowSize(Pina::Vector2(300, 0));
        Window window("Model Loader", nullptr, Pina::UIWindowFlags::AlwaysAutoResize);
        if (window) {
            // Model Selection
            if (CollapsingHeader header("Model Selection", Pina::UITreeNodeFlags::DefaultOpen); header) {
                int modelIdx = static_cast<int>(m_selectedModel);
                Combo modelCombo("Model", &modelIdx, "Gameboy\0Post-Apocalyptic\0Vehicle\0");
                if (modelCombo.changed()) {
                    ModelType newModel = static_cast<ModelType>(modelIdx);
                    if (newModel != m_selectedModel) {
                        m_selectedModel = newModel;
                        loadModel(m_selectedModel);
                    }
                }
            }

            Separator();

            // Model Info
            if (CollapsingHeader header("Model Info", Pina::UITreeNodeFlags::DefaultOpen); header) {
                if (m_model) {
                    Text{green, "Model loaded successfully"};
                    char meshBuf[32], matBuf[32], sizeBuf[64], pbrBuf[32];
                    snprintf(meshBuf, sizeof(meshBuf), "Meshes: %zu", m_model->getMeshCount());
                    snprintf(matBuf, sizeof(matBuf), "Materials: %zu", m_model->getMaterialCount());
                    snprintf(sizeBuf, sizeof(sizeBuf), "Size: %.2f x %.2f x %.2f",
                             m_model->getSize().x, m_model->getSize().y, m_model->getSize().z);
                    snprintf(pbrBuf, sizeof(pbrBuf), "Shader: %s", m_usePBR ? "PBR" : "Blinn-Phong");
                    { Text t1(meshBuf); }
                    { Text t2(matBuf); }
                    { Text t3(sizeBuf); }
                    { Text t4(pbrBuf); }
                } else {
                    Text{Pina::Color::red(), "Model failed to load"};
                    Text{"Using fallback cube"};
                }
            }

            Separator();

            // Camera Mode
            if (CollapsingHeader header("Camera", Pina::UITreeNodeFlags::DefaultOpen); header) {
                int cameraIdx = static_cast<int>(m_cameraMode);
                Combo cameraCombo("Mode", &cameraIdx, "Orbit\0Freelook\0");
                if (cameraCombo.changed()) {
                    CameraMode newMode = static_cast<CameraMode>(cameraIdx);
                    if (newMode != m_cameraMode) {
                        m_cameraMode = newMode;
                        if (m_cameraMode == CameraMode::Orbit) {
                            // Transfer camera state to orbit controller
                            m_orbitCamera->setTarget(glm::vec3(0.0f, 1.0f, 0.0f));
                        }
                    }
                }

                char posBuf[64];
                snprintf(posBuf, sizeof(posBuf), "Pos: (%.1f, %.1f, %.1f)",
                         m_camera.getPosition().x, m_camera.getPosition().y, m_camera.getPosition().z);
                { Text t1(posBuf); }

                if (m_cameraMode == CameraMode::Orbit) {
                    Text{"Controls: RMB=Rotate, MMB=Pan, Scroll=Zoom"};
                } else {
                    Text{"Controls: WASD=Move, RMB+Drag=Look"};
                }
            }

            Separator();

            // Transform
            if (CollapsingHeader header("Transform", Pina::UITreeNodeFlags::None); header) {
                Text{"Position:"};
                SliderFloat("X##pos", &m_modelPosition.x, -5.0f, 5.0f);
                SliderFloat("Y##pos", &m_modelPosition.y, -2.0f, 5.0f);
                SliderFloat("Z##pos", &m_modelPosition.z, -5.0f, 5.0f);

                Spacing();

                Text{"Rotation (degrees):"};
                SliderFloat("X##rot", &m_modelRotation.x, -180.0f, 180.0f);
                SliderFloat("Y##rot", &m_modelRotation.y, -180.0f, 180.0f);
                SliderFloat("Z##rot", &m_modelRotation.z, -180.0f, 180.0f);

                Spacing();

                if (Button("Reset All")) {
                    m_modelPosition = glm::vec3(0.0f);
                    m_modelRotation = glm::vec3(0.0f);
                }
            }

            Separator();

            // Rendering
            if (CollapsingHeader header("Rendering", Pina::UITreeNodeFlags::DefaultOpen); header) {
                Checkbox("[Space] Auto-Rotate", &m_autoRotate);
                Checkbox("[Z] Wireframe", &m_wireframe);
            }
        }
    }

    void onResize(int width, int height) override {
        m_device->setViewport(0, 0, width, height);
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }

    void onShutdown() override {
        m_orbitCamera.reset();
        m_freelookCamera.reset();
        m_shader.reset();
        m_pbrShader.reset();
        m_model.reset();
        m_fallbackCube.reset();
        m_lightCube.reset();
        m_device.reset();
    }

private:
    Pina::UNIQUE<Pina::GraphicsDevice> m_device;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::Shader> m_pbrShader;
    Pina::UNIQUE<Pina::Model> m_model;
    Pina::UNIQUE<Pina::CubeMesh> m_fallbackCube;
    Pina::UNIQUE<Pina::CubeMesh> m_lightCube;
    Pina::Camera m_camera;

    // Camera controllers
    std::unique_ptr<Pina::OrbitCamera> m_orbitCamera;
    std::unique_ptr<Pina::FreelookCamera> m_freelookCamera;
    CameraMode m_cameraMode = CameraMode::Orbit;

    // Lights
    Pina::LightManager m_lightManager;
    Pina::DirectionalLight m_sunLight;
    Pina::PointLight m_pointLight;
    Pina::PointLight m_backLight;

    // Camera state (for legacy/fallback)
    glm::vec3 m_cameraPos{0.0f, 2.0f, 5.0f};

    // Animation
    float m_autoRotation = 0.0f;
    bool m_autoRotate = false;

    // Rendering modes
    bool m_wireframe = false;
    bool m_usePBR = false;

    // Model selection
    ModelType m_selectedModel = ModelType::Gameboy;

    // Model transform controls
    glm::vec3 m_modelPosition{0.0f};
    glm::vec3 m_modelRotation{0.0f};
    float m_modelScale = 1.0f;
    glm::vec3 m_modelCenter{0.0f};
    float m_modelBaseY = 0.0f;
};

PINA_APPLICATION(ModelSample)
