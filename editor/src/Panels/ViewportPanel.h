#pragma once

#include "Panel.h"
#include <glm/glm.hpp>
#include <Pina.h>

namespace Pina {
    class Scene;
    class Shader;
    class Input;
    class GraphicsDevice;
}

namespace PinaEditor {

class Selection;
class EditorCamera;
class GizmoRenderer;
class TranslateGizmo;
class RotateGizmo;
class ScaleGizmo;
enum class GizmoMode;

/// Shading mode for viewport rendering
enum class ShadingMode {
    Smooth,     ///< Smooth shading with interpolated normals (default)
    Flat,       ///< Flat shading with face normals
    Wireframe   ///< Wireframe rendering
};

/// Viewport panel - 3D scene view with gizmos
class ViewportPanel : public Panel {
public:
    ViewportPanel(Pina::Scene* scene, Selection* selection, EditorCamera* camera);
    ~ViewportPanel();

    void onRender() override;
    void onUpdate(float deltaTime) override;

    void setScene(Pina::Scene* scene) { m_scene = scene; }
    void setInput(Pina::Input* input) { m_input = input; }
    void setShader(Pina::Shader* shader) { m_shader = shader; }
    void setGraphicsDevice(Pina::GraphicsDevice* device);

    // Gizmo control
    GizmoMode getGizmoMode() const;
    void setGizmoMode(GizmoMode mode);

    // Shading mode control
    ShadingMode getShadingMode() const { return m_shadingMode; }
    void setShadingMode(ShadingMode mode) { m_shadingMode = mode; }

    // Get viewport dimensions
    glm::vec2 getViewportSize() const { return m_viewportSize; }
    bool isViewportFocused() const { return m_viewportFocused; }
    bool isViewportHovered() const { return m_viewportHovered; }

private:
    void renderScene();
    void renderGizmos();

    Pina::Scene* m_scene = nullptr;
    Pina::Shader* m_shader = nullptr;
    Pina::Input* m_input = nullptr;
    Pina::GraphicsDevice* m_graphicsDevice = nullptr;
    Selection* m_selection = nullptr;
    EditorCamera* m_editorCamera = nullptr;

    std::unique_ptr<Pina::SceneRenderer> m_sceneRenderer;
    std::unique_ptr<GizmoRenderer> m_gizmoRenderer;
    std::unique_ptr<TranslateGizmo> m_translateGizmo;
    std::unique_ptr<RotateGizmo> m_rotateGizmo;
    std::unique_ptr<ScaleGizmo> m_scaleGizmo;

    GizmoMode m_gizmoMode;
    ShadingMode m_shadingMode = ShadingMode::Smooth;

    glm::vec2 m_viewportSize = glm::vec2(800, 600);
    bool m_viewportFocused = false;
    bool m_viewportHovered = false;

    // Framebuffer for viewport rendering
    uint32_t m_framebuffer = 0;
    uint32_t m_colorTexture = 0;
    uint32_t m_depthRenderbuffer = 0;
    glm::vec2 m_framebufferSize = glm::vec2(0, 0);

    void createFramebuffer(int width, int height);
    void deleteFramebuffer();
};

} // namespace PinaEditor
