/// Texture Sample
/// Demonstrates texture loading and rendering with the material system

#include <Pina.h>
#include <iostream>
#include <vector>

class TextureSample : public Pina::Application {
public:
    TextureSample() {
        m_config.title = "Pina Engine - Texture Sample";
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

        // Create procedural textures (since we don't have external files)
        createCheckerboardTexture();
        createBrickTexture();

        // Setup materials
        m_checkerMaterial = Pina::Material::createDefault();
        m_checkerMaterial.setDiffuseMap(m_checkerTexture.get());

        m_brickMaterial = Pina::Material::createPlastic(Pina::Color::white(), 16.0f);
        m_brickMaterial.setDiffuseMap(m_brickTexture.get());

        m_plainMaterial = Pina::Material::createMetal(Pina::Color(0.8f, 0.6f, 0.2f), 64.0f);

        // Create rotating center cube (uses selected texture)
        m_centerCube = m_scene.createCube("CenterCube", 1.0f);
        m_centerCube->setMaterial(m_checkerMaterial);

        // Create left cube (brick texture)
        m_brickCube = m_scene.createCube("BrickCube", 1.0f);
        m_brickCube->setMaterial(m_brickMaterial);
        m_brickCube->getTransform().setLocalPosition(-2.5f, 0.0f, 0.0f);

        // Create right cube (metal, no texture)
        m_metalCube = m_scene.createCube("MetalCube", 1.0f);
        m_metalCube->setMaterial(m_plainMaterial);
        m_metalCube->getTransform().setLocalPosition(2.5f, 0.0f, 0.0f);

        // Create floor plane (using a flat cube since we don't have createPlane yet)
        m_floor = m_scene.createCube("Floor", 1.0f);
        m_floor->setMaterial(m_checkerMaterial);
        m_floor->getTransform().setLocalPosition(0.0f, -1.5f, 0.0f);
        m_floor->getTransform().setLocalScale(6.0f, 0.1f, 6.0f);

        // Setup lights
        setupLights();

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

        std::cout << "=== Texture Sample ===" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD - Move camera" << std::endl;
        std::cout << "  Right Mouse + Drag - Look around" << std::endl;
        std::cout << "  1/2/3 - Switch texture mode" << std::endl;
        std::cout << "  Space - Toggle rotation" << std::endl;
        std::cout << "  Escape - Quit" << std::endl;
        std::cout << "======================" << std::endl;
    }

    void createCheckerboardTexture() {
        const int size = 256;
        const int squareSize = 32;
        std::vector<unsigned char> data(size * size * 3);

        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                int index = (y * size + x) * 3;
                bool isWhite = ((x / squareSize) + (y / squareSize)) % 2 == 0;
                unsigned char color = isWhite ? 255 : 50;
                data[index + 0] = color;
                data[index + 1] = color;
                data[index + 2] = color;
            }
        }

        m_checkerTexture = Pina::Texture::create(getDevice(), data.data(), size, size, 3);
        std::cout << "Created checkerboard texture (" << size << "x" << size << ")" << std::endl;
    }

    void createBrickTexture() {
        const int size = 256;
        std::vector<unsigned char> data(size * size * 3);

        // Brick pattern
        const int brickHeight = 32;
        const int brickWidth = 64;
        const int mortarSize = 4;

        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                int index = (y * size + x) * 3;

                // Determine brick row
                int row = y / brickHeight;
                int yInBrick = y % brickHeight;

                // Offset every other row
                int xOffset = (row % 2) * (brickWidth / 2);
                int adjustedX = (x + xOffset) % size;
                int xInBrick = adjustedX % brickWidth;

                // Check if we're in mortar
                bool isMortar = (yInBrick < mortarSize) || (xInBrick < mortarSize);

                if (isMortar) {
                    // Gray mortar
                    data[index + 0] = 128;
                    data[index + 1] = 128;
                    data[index + 2] = 128;
                } else {
                    // Brick color with slight variation
                    int variation = ((x * 7 + y * 13) % 30) - 15;
                    data[index + 0] = static_cast<unsigned char>(std::min(255, std::max(0, 180 + variation)));
                    data[index + 1] = static_cast<unsigned char>(std::min(255, std::max(0, 80 + variation / 2)));
                    data[index + 2] = static_cast<unsigned char>(std::min(255, std::max(0, 60 + variation / 3)));
                }
            }
        }

        m_brickTexture = Pina::Texture::create(getDevice(), data.data(), size, size, 3);
        std::cout << "Created brick texture (" << size << "x" << size << ")" << std::endl;
    }

    void setupLights() {
        auto& lightManager = m_scene.getLightManager();

        // Sun light
        m_sunLight.setDirection(Pina::Vector3(-0.5f, -1.0f, -0.3f));
        m_sunLight.setColor(Pina::Color(1.0f, 0.95f, 0.9f));
        m_sunLight.setIntensity(0.8f);
        m_sunLight.setAmbient(Pina::Color(0.1f, 0.1f, 0.12f));

        // Point light
        m_pointLight.setPosition(Pina::Vector3(2.0f, 2.0f, 2.0f));
        m_pointLight.setColor(Pina::Color(1.0f, 0.8f, 0.6f));
        m_pointLight.setIntensity(1.0f);
        m_pointLight.setRange(10.0f);

        lightManager.addLight(&m_sunLight);
        lightManager.addLight(&m_pointLight);
        lightManager.setGlobalAmbient(Pina::Color(0.05f, 0.05f, 0.07f));
    }

    void onUpdate(float deltaTime) override {
        auto* input = getInput();
        if (!input) return;

        if (input->isKeyPressed(Pina::Key::Escape)) {
            quit();
            return;
        }

        // Texture mode switching
        if (input->isKeyPressed(Pina::Key::Num1)) {
            m_textureMode = 0;
            m_centerCube->setMaterial(m_checkerMaterial);
            std::cout << "Mode: Checkerboard texture" << std::endl;
        }
        if (input->isKeyPressed(Pina::Key::Num2)) {
            m_textureMode = 1;
            m_centerCube->setMaterial(m_brickMaterial);
            std::cout << "Mode: Brick texture" << std::endl;
        }
        if (input->isKeyPressed(Pina::Key::Num3)) {
            m_textureMode = 2;
            m_centerCube->setMaterial(m_plainMaterial);
            std::cout << "Mode: No texture (plain material)" << std::endl;
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

        // Update camera
        if (auto* camera = m_scene.getActiveCamera()) {
            camera->lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
        }

        // Update rotation
        if (m_rotate) {
            m_rotation += deltaTime * 30.0f;
        }

        // Update cube rotations
        m_centerCube->getTransform().setLocalRotationEuler(m_rotation * 0.5f, m_rotation, 0.0f);
        m_brickCube->getTransform().setLocalRotationEuler(0.0f, m_rotation * 0.3f, 0.0f);
        m_metalCube->getTransform().setLocalRotationEuler(0.0f, -m_rotation * 0.4f, 0.0f);

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

        setNextWindowSize(Pina::Vector2(220, 0));
        Window window("Textures", nullptr, Pina::UIWindowFlags::AlwaysAutoResize);
        if (window) {
            if (CollapsingHeader header("Mode", Pina::UITreeNodeFlags::DefaultOpen); header) {
                Text(m_textureMode == 0 ? green : gray, "[1] Checkerboard");
                Text(m_textureMode == 1 ? green : gray, "[2] Brick");
                Text(m_textureMode == 2 ? green : gray, "[3] Plain");
            }

            Separator();

            if (CollapsingHeader header("Animation", Pina::UITreeNodeFlags::DefaultOpen); header) {
                Text(m_rotate ? green : gray, "[Space] Rotation");
            }

            Separator();

            if (CollapsingHeader header("Info", Pina::UITreeNodeFlags::DefaultOpen); header) {
                Text{"Center cube uses selected texture"};
                Text{"Left cube: Brick texture"};
                Text{"Right cube: Metal (no texture)"};
                Text{"Floor: Checkerboard"};
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
        m_checkerTexture.reset();
        m_brickTexture.reset();
    }

private:
    Pina::Scene m_scene;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::SceneRenderer> m_renderer;

    // Scene nodes
    Pina::Node* m_centerCube = nullptr;
    Pina::Node* m_brickCube = nullptr;
    Pina::Node* m_metalCube = nullptr;
    Pina::Node* m_floor = nullptr;

    // Textures
    Pina::UNIQUE<Pina::Texture> m_checkerTexture;
    Pina::UNIQUE<Pina::Texture> m_brickTexture;

    // Materials
    Pina::Material m_checkerMaterial;
    Pina::Material m_brickMaterial;
    Pina::Material m_plainMaterial;

    // Lights
    Pina::DirectionalLight m_sunLight;
    Pina::PointLight m_pointLight;

    // Camera state
    glm::vec3 m_cameraPos{0.0f, 2.0f, 6.0f};
    glm::vec3 m_cameraFront{0.0f, -0.3f, -1.0f};
    glm::vec3 m_cameraUp{0.0f, 1.0f, 0.0f};
    float m_yaw = -90.0f;
    float m_pitch = -15.0f;

    // Animation
    float m_rotation = 0.0f;
    bool m_rotate = true;
    int m_textureMode = 0;  // 0=checker, 1=brick, 2=plain
};

PINA_APPLICATION(TextureSample)
