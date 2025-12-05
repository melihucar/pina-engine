#pragma once

/// Pina Engine - Scene Renderer
/// Renders a scene by traversing the node hierarchy

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include <glm/glm.hpp>

namespace Pina {

class Scene;
class Node;
class Shader;
class Camera;
class GraphicsDevice;
class LightManager;

/// Renders a scene by traversing nodes and drawing attached models
class PINA_API SceneRenderer {
public:
    explicit SceneRenderer(GraphicsDevice* device);
    ~SceneRenderer();

    // ========================================================================
    // Rendering
    // ========================================================================

    /// Render the entire scene
    /// @param scene Scene to render
    /// @param shader Shader to use for rendering
    void render(Scene* scene, Shader* shader);

    /// Render a single node and its descendants
    /// @param node Node to render
    /// @param shader Shader to use
    /// @param camera Camera for view/projection
    /// @param lightManager Light manager for lighting
    void renderNode(Node* node, Shader* shader, Camera* camera, LightManager* lightManager);

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Enable/disable rendering of disabled nodes
    void setRenderDisabled(bool render) { m_renderDisabled = render; }
    bool getRenderDisabled() const { return m_renderDisabled; }

    /// Enable/disable wireframe mode
    void setWireframe(bool wireframe) { m_wireframe = wireframe; }
    bool getWireframe() const { return m_wireframe; }

    // ========================================================================
    // Statistics
    // ========================================================================

    /// Get number of nodes rendered in last frame
    size_t getRenderedNodeCount() const { return m_renderedNodeCount; }

    /// Get number of draw calls in last frame
    size_t getDrawCallCount() const { return m_drawCallCount; }

private:
    void renderNodeRecursive(Node* node, Shader* shader, Camera* camera, LightManager* lightManager);

    GraphicsDevice* m_device;

    bool m_renderDisabled = false;
    bool m_wireframe = false;

    // Per-frame statistics
    size_t m_renderedNodeCount = 0;
    size_t m_drawCallCount = 0;
};

} // namespace Pina
