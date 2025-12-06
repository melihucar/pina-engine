/// Model Sample
/// Demonstrates loading and rendering 3D models with PBR support

#include <Pina.h>
#include <iostream>
#include <cstdio>

enum class ModelType {
    Winter,
    Vehicle,
    PostApocalyptic
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
        m_config.clearColor = Pina::Color(0.1f, 0.1f, 0.12f);
    }

protected:
    void onInit() override {
        getDevice()->setDepthTest(true);

        // Setup scene
        m_scene.setDevice(getDevice());

        // Create shaders (standard Blinn-Phong and PBR)
        m_shader = getDevice()->createShader();
        if (!m_shader->load(Pina::ShaderLibrary::getStandardVertexShader(),
                            Pina::ShaderLibrary::getStandardFragmentShader())) {
            std::cerr << "Failed to compile standard shader!" << std::endl;
        }

        m_pbrShader = getDevice()->createShader();
        if (!m_pbrShader->load(Pina::ShaderLibrary::getPBRVertexShader(),
                               Pina::ShaderLibrary::getPBRFragmentShader())) {
            std::cerr << "Failed to compile PBR shader!" << std::endl;
        }

        // Create light cube for visualization
        m_frontLightMarker = m_scene.createCube("FrontLightMarker", 0.15f);
        m_backLightMarker = m_scene.createCube("BackLightMarker", 0.15f);

        // Setup camera with initial position
        auto* camera = m_scene.getOrCreateDefaultCamera(45.0f);
        camera->lookAt(m_cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Setup camera controllers
        m_orbitCamera = Pina::MAKE_UNIQUE<Pina::OrbitCamera>(camera);
        m_orbitCamera->setTarget(glm::vec3(0.0f, 1.0f, 0.0f));
        m_orbitCamera->setDistance(5.0f);
        m_orbitCamera->setRotation(-30.0f, 20.0f);

        m_freelookCamera = Pina::MAKE_UNIQUE<Pina::FreelookCamera>(camera);
        m_freelookCamera->setMoveSpeed(5.0f);

        // Load initial model
        loadModel(m_selectedModel);

        // Setup lights
        setupLights();

        // Create scene renderer
        m_renderer = Pina::MAKE_UNIQUE<Pina::SceneRenderer>(getDevice());

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
            case ModelType::Winter:
                path = "assets/winter/scene.gltf";
                break;
            case ModelType::Vehicle:
                path = "assets/vehicle/scene.gltf";
                break;
            case ModelType::PostApocalyptic:
                path = "assets/post_apocalyptic/scene.gltf";
                break;
        }

        // Remove existing model node if present
        if (m_modelNode) {
            // Note: Scene doesn't have removeNode yet, so we disable it
            m_modelNode->setEnabled(false);
        }

        // Create model node using Scene API
        m_modelNode = m_scene.createModel(path);
        if (!m_modelNode) {
            std::cerr << "Failed to load model: " << path << std::endl;
            // Create a fallback cube
            m_modelNode = m_scene.createCube("FallbackCube", 1.0f);
            m_modelNode->setMaterial(Pina::Material::createPlastic(Pina::Color::red(), 32.0f));
            m_modelScale = 1.0f;
            m_modelCenter = glm::vec3(0.0f);
            m_modelBaseY = 0.5f;
            m_usePBR = false;
        } else {
            // Get model info from the node's attached model
            auto* model = m_modelNode->getModel();
            if (model) {
                m_modelScale = model->calculateFitScale(2.0f);
                m_modelCenter = model->getCenter();
                m_modelBaseY = -model->getBoundingBox().min.y * m_modelScale;
                m_usePBR = model->hasPBRMaterials();

                std::cout << "Loaded model: " << path << std::endl;
                std::cout << "  Meshes: " << model->getMeshCount() << std::endl;
                std::cout << "  Materials: " << model->getMaterialCount() << std::endl;
                std::cout << "  PBR Materials: " << (m_usePBR ? "Yes" : "No") << std::endl;
                std::cout << "  Size: " << model->getSize().x << " x "
                          << model->getSize().y << " x " << model->getSize().z << std::endl;
                std::cout << "  Auto-fit scale: " << m_modelScale << std::endl;
            }
        }

        // Reset transform
        m_modelPosition = glm::vec3(0.0f);
        m_modelRotation = glm::vec3(0.0f);
        m_autoRotation = 0.0f;

        // Reset camera to best view of the model
        if (m_modelNode && m_modelNode->getModel() && m_orbitCamera) {
            auto* model = m_modelNode->getModel();
            glm::vec3 center = model->getCenter() * m_modelScale;
            center.y += m_modelBaseY;
            float size = model->getBoundingBox().getMaxDimension() * m_modelScale;
            m_orbitCamera->focusOn(center, size);
        }
    }

    void setupLights() {
        auto& lightManager = m_scene.getLightManager();

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

        lightManager.addLight(&m_sunLight);
        lightManager.addLight(&m_pointLight);
        lightManager.addLight(&m_backLight);
        lightManager.setGlobalAmbient(Pina::Color(0.15f, 0.15f, 0.18f));

        // Setup light markers
        Pina::Vector3 frontPos = m_pointLight.getPosition();
        m_frontLightMarker->getTransform().setLocalPosition(frontPos.x, frontPos.y, frontPos.z);
        m_frontLightMarker->setMaterial(Pina::Material::createEmissive(m_pointLight.getColor(), 1.0f));

        Pina::Vector3 backPos = m_backLight.getPosition();
        m_backLightMarker->getTransform().setLocalPosition(backPos.x, backPos.y, backPos.z);
        m_backLightMarker->setMaterial(Pina::Material::createEmissive(m_backLight.getColor(), 1.0f));
    }

    void onUpdate(float deltaTime) override {
        // Update FPS counter
        m_fpsAccumulator += deltaTime;
        m_frameCount++;
        if (m_fpsAccumulator >= 0.5f) {
            m_fps = m_frameCount / m_fpsAccumulator;
            m_fpsAccumulator = 0.0f;
            m_frameCount = 0;
        }

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
        auto* camera = m_scene.getActiveCamera();
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

        // Update model transform
        if (m_modelNode) {
            auto& transform = m_modelNode->getTransform();

            // Position offset (including floor placement)
            glm::vec3 pos = m_modelPosition + glm::vec3(0.0f, m_modelBaseY, 0.0f);
            transform.setLocalPosition(pos.x, pos.y, pos.z);

            // Combined rotation (auto + manual)
            transform.setLocalRotationEuler(
                m_modelRotation.x,
                m_modelRotation.y + m_autoRotation,
                m_modelRotation.z
            );

            // Scale to fit
            transform.setLocalScale(m_modelScale, m_modelScale, m_modelScale);
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

        // Select shader based on material type
        Pina::Shader* activeShader = m_usePBR ? m_pbrShader.get() : m_shader.get();

        // Apply wireframe mode if enabled
        getDevice()->setWireframe(m_wireframe);
        m_renderer->setWireframe(m_wireframe);

        // Render scene - but we need special handling for models with transparency
        // For now, render light markers with standard renderer
        activeShader->bind();
        auto* camera = m_scene.getActiveCamera();
        if (camera) {
            activeShader->setMat4("uView", camera->getViewMatrix());
            activeShader->setMat4("uProjection", camera->getProjectionMatrix());
        }

        m_scene.getLightManager().setViewPosition(camera->getPosition());
        m_scene.getLightManager().uploadToShader(activeShader);

        if (m_usePBR) {
            activeShader->setInt("uShadingMode", m_wireframe ? 2 : 0);
        } else {
            activeShader->setInt("uWireframe", m_wireframe ? 1 : 0);
        }

        // Draw model with two-pass rendering for correct transparency
        if (m_modelNode && m_modelNode->getModel()) {
            auto* model = m_modelNode->getModel();
            glm::mat4 modelMatrix = m_modelNode->getTransform().getWorldMatrix();
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
            activeShader->setMat4("uModel", modelMatrix);
            activeShader->setMat3("uNormalMatrix", normalMatrix);

            // Pass 1: Draw opaque meshes
            model->drawOpaque(activeShader, &m_scene.getLightManager());

            // Pass 2: Draw transparent meshes
            if (model->hasTransparentMaterials()) {
                getDevice()->setBlending(true);
                getDevice()->setDepthWrite(false);
                model->drawTransparent(activeShader, &m_scene.getLightManager());
                getDevice()->setDepthWrite(true);
                getDevice()->setBlending(false);
            }
        } else if (m_modelNode && m_modelNode->hasMesh()) {
            // Fallback cube
            glm::mat4 modelMatrix = m_modelNode->getTransform().getWorldMatrix();
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
            activeShader->setMat4("uModel", modelMatrix);
            activeShader->setMat3("uNormalMatrix", normalMatrix);
            m_scene.getLightManager().uploadMaterial(activeShader, m_modelNode->getMaterial());
            m_modelNode->getMesh()->draw();
        }

        // Draw light markers (use standard shader)
        m_shader->bind();
        m_shader->setMat4("uView", camera->getViewMatrix());
        m_shader->setMat4("uProjection", camera->getProjectionMatrix());
        m_shader->setInt("uWireframe", 0);
        m_scene.getLightManager().uploadToShader(m_shader.get());

        // Front light marker
        {
            glm::mat4 model = m_frontLightMarker->getTransform().getWorldMatrix();
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", glm::mat3(1.0f));
            m_scene.getLightManager().uploadMaterial(m_shader.get(), m_frontLightMarker->getMaterial());
            if (m_frontLightMarker->hasMesh()) {
                m_frontLightMarker->getMesh()->draw();
            }
        }

        // Back light marker
        {
            glm::mat4 model = m_backLightMarker->getTransform().getWorldMatrix();
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", glm::mat3(1.0f));
            m_scene.getLightManager().uploadMaterial(m_shader.get(), m_backLightMarker->getMaterial());
            if (m_backLightMarker->hasMesh()) {
                m_backLightMarker->getMesh()->draw();
            }
        }

        // Reset to solid mode for UI rendering
        getDevice()->setWireframe(false);

        getDevice()->endFrame();
    }

    void onRenderUI() override {
        using namespace Pina::Widgets;

        // FPS counter in top right corner
        {
            auto* window = getWindow();
            int windowWidth = window ? window->getWidth() : 1280;
            setNextWindowPos(Pina::Vector2(static_cast<float>(windowWidth) - 100.0f, 10.0f));
            Window fpsWindow("##fps", nullptr,
                Pina::UIWindowFlags::NoTitleBar |
                Pina::UIWindowFlags::NoResize |
                Pina::UIWindowFlags::NoMove |
                Pina::UIWindowFlags::NoBackground |
                Pina::UIWindowFlags::AlwaysAutoResize);
            if (fpsWindow) {
                char fpsBuf[32];
                snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %.0f", m_fps);
                Text{Pina::Color::white(), fpsBuf};
            }
        }

        Pina::Color green = Pina::Color::green();

        setNextWindowSize(Pina::Vector2(300, 0));
        Window window("Model Loader", nullptr, Pina::UIWindowFlags::AlwaysAutoResize);
        if (window) {
            // Model Selection
            if (CollapsingHeader header("Model Selection", Pina::UITreeNodeFlags::DefaultOpen); header) {
                int modelIdx = static_cast<int>(m_selectedModel);
                Combo modelCombo("Model", &modelIdx, "Winter\0Vehicle\0Post-Apocalyptic\0");
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
                if (m_modelNode && m_modelNode->getModel()) {
                    auto* model = m_modelNode->getModel();
                    Text{green, "Model loaded successfully"};
                    char meshBuf[32], matBuf[32], sizeBuf[64], pbrBuf[32];
                    snprintf(meshBuf, sizeof(meshBuf), "Meshes: %zu", model->getMeshCount());
                    snprintf(matBuf, sizeof(matBuf), "Materials: %zu", model->getMaterialCount());
                    snprintf(sizeBuf, sizeof(sizeBuf), "Size: %.2f x %.2f x %.2f",
                             model->getSize().x, model->getSize().y, model->getSize().z);
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
                            m_orbitCamera->setTarget(glm::vec3(0.0f, 1.0f, 0.0f));
                        }
                    }
                }

                auto* camera = m_scene.getActiveCamera();
                char posBuf[64];
                snprintf(posBuf, sizeof(posBuf), "Pos: (%.1f, %.1f, %.1f)",
                         camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
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
        getDevice()->setViewport(0, 0, width, height);
        if (auto* camera = m_scene.getActiveCamera()) {
            camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
        }
    }

    void onShutdown() override {
        m_orbitCamera.reset();
        m_freelookCamera.reset();
        m_renderer.reset();
        m_shader.reset();
        m_pbrShader.reset();
    }

private:
    Pina::Scene m_scene;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::Shader> m_pbrShader;
    Pina::UNIQUE<Pina::SceneRenderer> m_renderer;

    // Scene nodes
    Pina::Node* m_modelNode = nullptr;
    Pina::Node* m_frontLightMarker = nullptr;
    Pina::Node* m_backLightMarker = nullptr;

    // Camera controllers
    Pina::UNIQUE<Pina::OrbitCamera> m_orbitCamera;
    Pina::UNIQUE<Pina::FreelookCamera> m_freelookCamera;
    CameraMode m_cameraMode = CameraMode::Orbit;

    // Lights
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
    ModelType m_selectedModel = ModelType::Winter;

    // FPS tracking
    float m_fps = 0.0f;
    float m_fpsAccumulator = 0.0f;
    int m_frameCount = 0;

    // Model transform controls
    glm::vec3 m_modelPosition{0.0f};
    glm::vec3 m_modelRotation{0.0f};
    float m_modelScale = 1.0f;
    glm::vec3 m_modelCenter{0.0f};
    float m_modelBaseY = 0.0f;
};

PINA_APPLICATION(ModelSample)
