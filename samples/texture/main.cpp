/// Texture Sample
/// Demonstrates texture loading and rendering with the material system

#include <Pina.h>
#include <glm/gtc/matrix_transform.hpp>
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
    }

protected:
    void onInit() override {
        m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);
        m_device->setDepthTest(true);

        // Create meshes
        m_cube = Pina::CubeMesh::create(m_device.get(), 1.0f);
        m_quad = Pina::QuadMesh::create(m_device.get(), 2.0f, 2.0f);

        // Load standard lit shader
        m_shader = m_device->createShader();
        if (!m_shader->load(Pina::ShaderLibrary::getStandardVertexShader(),
                            Pina::ShaderLibrary::getStandardFragmentShader())) {
            std::cerr << "Failed to compile shader!" << std::endl;
        }

        // Create procedural textures (since we don't have external files)
        createCheckerboardTexture();
        createBrickTexture();

        // Setup materials
        m_checkerMaterial = Pina::Material::createDefault();
        m_checkerMaterial.setDiffuseMap(m_checkerTexture.get());

        m_brickMaterial = Pina::Material::createPlastic(Pina::Color::white(), 16.0f);
        m_brickMaterial.setDiffuseMap(m_brickTexture.get());

        m_plainMaterial = Pina::Material::createMetal(Pina::Color(0.8f, 0.6f, 0.2f), 64.0f);

        // Setup camera
        m_camera.setPerspective(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_camera.lookAt(m_cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Setup lights
        setupLights();

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

        m_checkerTexture = Pina::Texture::create(m_device.get(), data.data(), size, size, 3);
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

        m_brickTexture = Pina::Texture::create(m_device.get(), data.data(), size, size, 3);
        std::cout << "Created brick texture (" << size << "x" << size << ")" << std::endl;
    }

    void setupLights() {
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

        // Texture mode switching
        if (input->isKeyPressed(Pina::Key::Num1)) {
            m_textureMode = 0;
            std::cout << "Mode: Checkerboard texture" << std::endl;
        }
        if (input->isKeyPressed(Pina::Key::Num2)) {
            m_textureMode = 1;
            std::cout << "Mode: Brick texture" << std::endl;
        }
        if (input->isKeyPressed(Pina::Key::Num3)) {
            m_textureMode = 2;
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

        m_camera.lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);

        // Rotation
        if (m_rotate) {
            m_rotation += deltaTime * 30.0f;
        }

        m_lightManager.update();
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.1f, 0.1f, 0.12f);

        m_shader->bind();

        // Set view/projection matrices
        m_shader->setMat4("uView", m_camera.getViewMatrix());
        m_shader->setMat4("uProjection", m_camera.getProjectionMatrix());

        // Upload lighting
        m_lightManager.setViewPosition(m_cameraPos);
        m_lightManager.uploadToShader(m_shader.get());

        // Select current material based on mode
        Pina::Material* currentMaterial = nullptr;
        switch (m_textureMode) {
            case 0: currentMaterial = &m_checkerMaterial; break;
            case 1: currentMaterial = &m_brickMaterial; break;
            case 2: currentMaterial = &m_plainMaterial; break;
        }

        // Draw rotating cube
        m_lightManager.uploadMaterial(m_shader.get(), *currentMaterial);
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(m_rotation * 0.5f), glm::vec3(1.0f, 0.0f, 0.0f));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", normalMatrix);
            m_cube->draw();
        }

        // Draw floor (with checkerboard regardless of mode)
        m_lightManager.uploadMaterial(m_shader.get(), m_checkerMaterial);
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(3.0f, 3.0f, 1.0f));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", normalMatrix);
            m_quad->draw();
        }

        // Draw second cube with brick texture
        m_lightManager.uploadMaterial(m_shader.get(), m_brickMaterial);
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-2.5f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(m_rotation * 0.3f), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", normalMatrix);
            m_cube->draw();
        }

        // Draw third cube without texture
        m_lightManager.uploadMaterial(m_shader.get(), m_plainMaterial);
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-m_rotation * 0.4f), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            m_shader->setMat4("uModel", model);
            m_shader->setMat3("uNormalMatrix", normalMatrix);
            m_cube->draw();
        }

        m_device->endFrame();
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
        m_device->setViewport(0, 0, width, height);
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }

    void onShutdown() override {
        m_shader.reset();
        m_cube.reset();
        m_quad.reset();
        m_checkerTexture.reset();
        m_brickTexture.reset();
        m_device.reset();
    }

private:
    Pina::UNIQUE<Pina::GraphicsDevice> m_device;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::CubeMesh> m_cube;
    Pina::UNIQUE<Pina::QuadMesh> m_quad;
    Pina::Camera m_camera;

    // Textures
    Pina::UNIQUE<Pina::Texture> m_checkerTexture;
    Pina::UNIQUE<Pina::Texture> m_brickTexture;

    // Materials
    Pina::Material m_checkerMaterial;
    Pina::Material m_brickMaterial;
    Pina::Material m_plainMaterial;

    // Lights
    Pina::LightManager m_lightManager;
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
