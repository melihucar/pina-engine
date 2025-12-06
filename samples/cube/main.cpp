/// Cube Sample
/// Demonstrates 3D rendering with a rotating cube using the simplified API

#include <Pina.h>

class CubeSample : public Pina::Application {
public:
    CubeSample() {
        m_config.title = "Pina Engine - Rotating Cube";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
        m_config.vsync = true;
        m_config.resizable = true;
        m_config.clearColor = Pina::Color(0.1f, 0.1f, 0.15f);
    }

protected:
    void onInit() override {
        // Setup scene with device
        m_scene.setDevice(getDevice());

        // Setup default lighting
        m_scene.setupDefaultLighting();

        // Create a cube using the simplified API
        m_cube = m_scene.createCube("MyCube", 1.0f);
        m_cube->setMaterial(Pina::Material::createPlastic(Pina::Color(0.8f, 0.3f, 0.2f)));

        // Setup camera
        auto* camera = m_scene.getOrCreateDefaultCamera(45.0f);
        camera->lookAt(
            glm::vec3(2.0f, 2.0f, 3.0f),  // Position
            glm::vec3(0.0f, 0.0f, 0.0f),  // Target
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up
        );

        // Create shader from ShaderLibrary
        m_shader = getDevice()->createShader();
        m_shader->load(
            Pina::ShaderLibrary::getStandardVertexShader(),
            Pina::ShaderLibrary::getStandardFragmentShader()
        );

        // Create scene renderer
        m_renderer = Pina::MAKE_UNIQUE<Pina::SceneRenderer>(getDevice());

        // Enable depth testing
        getDevice()->setDepthTest(true);
    }

    void onUpdate(float deltaTime) override {
        // Rotate cube
        m_rotation += deltaTime * 50.0f;
        if (m_rotation > 360.0f) {
            m_rotation -= 360.0f;
        }

        // Update cube transform
        auto& transform = m_cube->getTransform();
        transform.setLocalRotationEuler(m_rotation * 0.5f, m_rotation, 0.0f);

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

        // Render entire scene with one call
        m_renderer->render(&m_scene, m_shader.get());

        getDevice()->endFrame();
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
    Pina::Node* m_cube = nullptr;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::UNIQUE<Pina::SceneRenderer> m_renderer;
    float m_rotation = 0.0f;
};

PINA_APPLICATION(CubeSample)
