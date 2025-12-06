#pragma once

/// Pina Engine - Scene
/// Container for a scene hierarchy with root node, camera, and lights

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include "../Graphics/Lighting/LightManager.h"
#include "../Graphics/Lighting/DirectionalLight.h"
#include "../Graphics/Lighting/PointLight.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Primitives/StaticMesh.h"
#include "Node.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <utility>

namespace Pina {

class Input;
class GraphicsDevice;
class Model;

/// Scene container for 3D objects, camera, and lights
class PINA_API Scene {
public:
    Scene();
    ~Scene();

    // Non-copyable
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // ========================================================================
    // Root Node
    // ========================================================================

    /// Get the root node of the scene
    Node* getRoot() { return m_root.get(); }
    const Node* getRoot() const { return m_root.get(); }

    // ========================================================================
    // Node Creation
    // ========================================================================

    /// Create a new node as child of root
    /// @param name Node name
    /// @return Pointer to newly created node
    Node* createNode(const std::string& name = "Node");

    /// Create a new node as child of specified parent
    /// @param name Node name
    /// @param parent Parent node (nullptr = root)
    /// @return Pointer to newly created node
    Node* createNode(const std::string& name, Node* parent);

    // ========================================================================
    // Node Lookup
    // ========================================================================

    /// Find a node by ID
    /// @param id Node ID
    /// @return Node pointer or nullptr if not found
    Node* findNode(uint64_t id) const;

    /// Find a node by name (first match)
    /// @param name Node name
    /// @return Node pointer or nullptr if not found
    Node* findNode(const std::string& name) const;

    /// Get total number of nodes in scene
    size_t getNodeCount() const { return m_nodesByID.size(); }

    // ========================================================================
    // Traversal
    // ========================================================================

    /// Traverse all nodes in the scene
    void traverse(const std::function<void(Node*)>& callback);

    /// Traverse all nodes (const version)
    void traverse(const std::function<void(const Node*)>& callback) const;

    /// Traverse only enabled nodes
    void traverseEnabled(const std::function<void(Node*)>& callback);

    // ========================================================================
    // Camera Management
    // ========================================================================

    /// Add a camera with a name (takes ownership)
    /// @param name Camera name for lookup
    /// @param camera Camera to add
    void addCamera(const std::string& name, UNIQUE<Camera> camera);

    /// Add a camera with a name (creates a new one)
    /// @tparam T Camera type (must derive from Camera)
    /// @tparam Args Constructor argument types
    /// @param name Camera name for lookup
    /// @param args Constructor arguments
    /// @return Pointer to the created camera
    template<typename T, typename... Args>
    T* addCamera(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of<Camera, T>::value, "T must derive from Camera");
        auto camera = MAKE_UNIQUE<T>(std::forward<Args>(args)...);
        T* ptr = camera.get();
        addCamera(name, std::move(camera));
        return ptr;
    }

    /// Get a camera by name
    /// @param name Camera name
    /// @return Camera pointer or nullptr if not found
    Camera* getCamera(const std::string& name) const;

    /// Get a camera by name with type casting
    template<typename T>
    T* getCamera(const std::string& name) const {
        return dynamic_cast<T*>(getCamera(name));
    }

    /// Remove a camera by name
    /// @param name Camera name
    void removeCamera(const std::string& name);

    /// Set the active camera by name
    /// @param name Camera name
    /// @return true if camera was found and set
    bool setActiveCamera(const std::string& name);

    /// Set the active camera directly
    void setActiveCamera(Camera* camera) { m_activeCamera = camera; }

    /// Get the active camera
    Camera* getActiveCamera() const { return m_activeCamera; }

    /// Update camera input for the active camera
    /// @param input Input system
    /// @param deltaTime Time since last frame
    void updateCameraInput(Input* input, float deltaTime);

    /// Get or create a default camera
    /// @param fov Field of view in degrees
    /// @return Pointer to the default camera
    Camera* getOrCreateDefaultCamera(float fov = 45.0f);

    // ========================================================================
    // Primitive Helpers
    // ========================================================================

    /// Create a cube node with a mesh
    /// @param name Node name
    /// @param size Cube size (edge length)
    /// @return Pointer to the created node
    Node* createCube(const std::string& name, float size = 1.0f);

    /// Create a sphere node with a mesh
    /// @param name Node name
    /// @param radius Sphere radius
    /// @param segments Number of latitude/longitude segments
    /// @return Pointer to the created node
    Node* createSphere(const std::string& name, float radius = 0.5f, int segments = 32);

    /// Create a plane node with a mesh
    /// @param name Node name
    /// @param width Plane width
    /// @param height Plane height
    /// @return Pointer to the created node
    Node* createPlane(const std::string& name, float width = 1.0f, float height = 1.0f);

    /// Create a node with a model loaded from file
    /// @param path Path to the model file
    /// @param name Node name (uses filename if empty)
    /// @return Pointer to the created node, or nullptr on failure
    Node* createModel(const std::string& path, const std::string& name = "");

    /// Setup default lighting (directional + ambient)
    void setupDefaultLighting();

    // ========================================================================
    // Lighting
    // ========================================================================

    /// Get the light manager
    LightManager& getLightManager() { return m_lightManager; }
    const LightManager& getLightManager() const { return m_lightManager; }

    // ========================================================================
    // Graphics Device
    // ========================================================================

    /// Set the graphics device (required for creating meshes)
    void setDevice(GraphicsDevice* device) { m_device = device; }

    /// Get the graphics device
    GraphicsDevice* getDevice() const { return m_device; }

    // ========================================================================
    // Update
    // ========================================================================

    /// Update the scene (called each frame)
    /// Updates light manager and other per-frame state
    void update(float deltaTime);

    // ========================================================================
    // Node Registry (Internal)
    // ========================================================================

    /// Register a node with the scene (called by Node)
    void registerNode(Node* node);

    /// Unregister a node from the scene (called by Node)
    void unregisterNode(Node* node);

private:
    UNIQUE<Node> m_root;
    Camera* m_activeCamera = nullptr;
    LightManager m_lightManager;
    GraphicsDevice* m_device = nullptr;

    // Node lookup by ID
    std::unordered_map<uint64_t, Node*> m_nodesByID;

    // Camera storage (named cameras owned by scene)
    std::unordered_map<std::string, UNIQUE<Camera>> m_cameras;

    // Owned primitive meshes (for createCube, createSphere, etc.)
    std::vector<UNIQUE<StaticMesh>> m_primitiveMeshes;

    // Owned models (for createModel)
    std::vector<UNIQUE<Model>> m_models;

    // Owned lights (for setupDefaultLighting)
    std::vector<UNIQUE<DirectionalLight>> m_ownedDirectionalLights;
    std::vector<UNIQUE<PointLight>> m_ownedPointLights;
};

} // namespace Pina
