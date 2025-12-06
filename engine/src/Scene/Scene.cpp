#include "Scene.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Model.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/Lighting/DirectionalLight.h"
#include <cmath>

namespace Pina {

Scene::Scene() {
    // Create root node
    m_root = MAKE_UNIQUE<Node>("Root");
    m_root->setScene(this);
}

Scene::~Scene() = default;

// ============================================================================
// Node Creation
// ============================================================================

Node* Scene::createNode(const std::string& name) {
    return createNode(name, m_root.get());
}

Node* Scene::createNode(const std::string& name, Node* parent) {
    Node* parentNode = parent ? parent : m_root.get();
    return parentNode->addChild(name);
}

// ============================================================================
// Node Lookup
// ============================================================================

Node* Scene::findNode(uint64_t id) const {
    auto it = m_nodesByID.find(id);
    if (it != m_nodesByID.end()) {
        return it->second;
    }
    return nullptr;
}

Node* Scene::findNode(const std::string& name) const {
    return m_root->findDescendant(name);
}

// ============================================================================
// Traversal
// ============================================================================

void Scene::traverse(const std::function<void(Node*)>& callback) {
    m_root->traverse(callback);
}

void Scene::traverse(const std::function<void(const Node*)>& callback) const {
    const Node* root = m_root.get();
    root->traverse(callback);
}

void Scene::traverseEnabled(const std::function<void(Node*)>& callback) {
    m_root->traverseEnabled(callback);
}

// ============================================================================
// Update
// ============================================================================

void Scene::update(float deltaTime) {
    (void)deltaTime;

    // Update light manager with camera position for specular calculations
    if (m_activeCamera) {
        m_lightManager.setViewPosition(m_activeCamera->getPosition());
    }

    // Update light data
    m_lightManager.update();
}

// ============================================================================
// Camera Management
// ============================================================================

void Scene::addCamera(const std::string& name, UNIQUE<Camera> camera) {
    if (camera) {
        m_cameras[name] = std::move(camera);
    }
}

Camera* Scene::getCamera(const std::string& name) const {
    auto it = m_cameras.find(name);
    return (it != m_cameras.end()) ? it->second.get() : nullptr;
}

void Scene::removeCamera(const std::string& name) {
    auto it = m_cameras.find(name);
    if (it != m_cameras.end()) {
        // If removing the active camera, clear it
        if (m_activeCamera == it->second.get()) {
            m_activeCamera = nullptr;
        }
        m_cameras.erase(it);
    }
}

bool Scene::setActiveCamera(const std::string& name) {
    Camera* camera = getCamera(name);
    if (camera) {
        m_activeCamera = camera;
        return true;
    }
    return false;
}

void Scene::updateCameraInput(Input* input, float deltaTime) {
    if (m_activeCamera && m_activeCamera->wantsInput()) {
        m_activeCamera->handleInput(input, deltaTime);
    }
}

Camera* Scene::getOrCreateDefaultCamera(float fov) {
    const std::string defaultName = "default";
    Camera* camera = getCamera(defaultName);
    if (!camera) {
        camera = addCamera<Camera>(defaultName);
        camera->setPerspective(fov, 16.0f / 9.0f, 0.1f, 100.0f);
        camera->lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        m_activeCamera = camera;
    }
    return camera;
}

// ============================================================================
// Primitive Helpers
// ============================================================================

Node* Scene::createCube(const std::string& name, float size) {
    if (!m_device) return nullptr;

    // Cube vertex data (position, normal, texcoord)
    float h = size * 0.5f;
    std::vector<float> vertices = {
        // Front face
        -h, -h,  h,  0, 0, 1,  0, 0,
         h, -h,  h,  0, 0, 1,  1, 0,
         h,  h,  h,  0, 0, 1,  1, 1,
        -h,  h,  h,  0, 0, 1,  0, 1,
        // Back face
         h, -h, -h,  0, 0,-1,  0, 0,
        -h, -h, -h,  0, 0,-1,  1, 0,
        -h,  h, -h,  0, 0,-1,  1, 1,
         h,  h, -h,  0, 0,-1,  0, 1,
        // Top face
        -h,  h,  h,  0, 1, 0,  0, 0,
         h,  h,  h,  0, 1, 0,  1, 0,
         h,  h, -h,  0, 1, 0,  1, 1,
        -h,  h, -h,  0, 1, 0,  0, 1,
        // Bottom face
        -h, -h, -h,  0,-1, 0,  0, 0,
         h, -h, -h,  0,-1, 0,  1, 0,
         h, -h,  h,  0,-1, 0,  1, 1,
        -h, -h,  h,  0,-1, 0,  0, 1,
        // Right face
         h, -h,  h,  1, 0, 0,  0, 0,
         h, -h, -h,  1, 0, 0,  1, 0,
         h,  h, -h,  1, 0, 0,  1, 1,
         h,  h,  h,  1, 0, 0,  0, 1,
        // Left face
        -h, -h, -h, -1, 0, 0,  0, 0,
        -h, -h,  h, -1, 0, 0,  1, 0,
        -h,  h,  h, -1, 0, 0,  1, 1,
        -h,  h, -h, -1, 0, 0,  0, 1,
    };
    std::vector<uint32_t> indices = {
        0, 1, 2, 0, 2, 3,       // Front
        4, 5, 6, 4, 6, 7,       // Back
        8, 9,10, 8,10,11,       // Top
        12,13,14,12,14,15,      // Bottom
        16,17,18,16,18,19,      // Right
        20,21,22,20,22,23       // Left
    };

    auto mesh = StaticMesh::create(m_device, vertices, indices);
    if (!mesh) return nullptr;

    Node* node = createNode(name);
    node->setMesh(mesh.get());
    m_primitiveMeshes.push_back(std::move(mesh));
    return node;
}

Node* Scene::createSphere(const std::string& name, float radius, int segments) {
    if (!m_device) return nullptr;

    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    // Generate sphere vertices
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * 3.14159265f / segments;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * 3.14159265f / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            // Position
            vertices.push_back(x * radius);
            vertices.push_back(y * radius);
            vertices.push_back(z * radius);
            // Normal (same as position for unit sphere)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            // TexCoord
            vertices.push_back(static_cast<float>(lon) / segments);
            vertices.push_back(static_cast<float>(lat) / segments);
        }
    }

    // Generate indices
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            uint32_t first = lat * (segments + 1) + lon;
            uint32_t second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    auto mesh = StaticMesh::create(m_device, vertices, indices);
    if (!mesh) return nullptr;

    Node* node = createNode(name);
    node->setMesh(mesh.get());
    m_primitiveMeshes.push_back(std::move(mesh));
    return node;
}

Node* Scene::createPlane(const std::string& name, float width, float height) {
    if (!m_device) return nullptr;

    float hw = width * 0.5f;
    float hh = height * 0.5f;

    std::vector<float> vertices = {
        // Position        Normal       TexCoord
        -hw, 0, -hh,   0, 1, 0,   0, 0,
         hw, 0, -hh,   0, 1, 0,   1, 0,
         hw, 0,  hh,   0, 1, 0,   1, 1,
        -hw, 0,  hh,   0, 1, 0,   0, 1,
    };
    std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

    auto mesh = StaticMesh::create(m_device, vertices, indices);
    if (!mesh) return nullptr;

    Node* node = createNode(name);
    node->setMesh(mesh.get());
    m_primitiveMeshes.push_back(std::move(mesh));
    return node;
}

Node* Scene::createModel(const std::string& path, const std::string& name) {
    if (!m_device) return nullptr;

    auto model = Model::load(m_device, path);
    if (!model) return nullptr;

    // Extract name from path if not provided
    std::string nodeName = name;
    if (nodeName.empty()) {
        size_t lastSlash = path.find_last_of("/\\");
        size_t lastDot = path.find_last_of('.');
        if (lastSlash != std::string::npos) {
            nodeName = path.substr(lastSlash + 1, lastDot - lastSlash - 1);
        } else {
            nodeName = path.substr(0, lastDot);
        }
    }

    Node* node = createNode(nodeName);
    node->setModel(model.get());
    m_models.push_back(std::move(model));
    return node;
}

void Scene::setupDefaultLighting() {
    // Clear existing lights
    m_lightManager.clear();
    m_ownedDirectionalLights.clear();
    m_ownedPointLights.clear();

    // Create and add a directional light (sun-like)
    auto dirLight = MAKE_UNIQUE<DirectionalLight>();
    dirLight->setDirection(glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f)));
    dirLight->setColor(Color(1.0f, 0.98f, 0.95f));  // Slightly warm
    dirLight->setIntensity(1.0f);
    m_lightManager.addLight(dirLight.get());
    m_ownedDirectionalLights.push_back(std::move(dirLight));

    // Set global ambient color (slightly cool)
    m_lightManager.setGlobalAmbient(Color(0.2f, 0.2f, 0.25f));
}

// ============================================================================
// Node Registry
// ============================================================================

void Scene::registerNode(Node* node) {
    if (node) {
        m_nodesByID[node->getID()] = node;
    }
}

void Scene::unregisterNode(Node* node) {
    if (node) {
        m_nodesByID.erase(node->getID());
    }
}

} // namespace Pina
