#pragma once

#include <Pina.h>
#include <memory>

namespace PinaEditor {

class Selection;
class EditorCamera;
class HierarchyPanel;
class InspectorPanel;
class ViewportPanel;
enum class GizmoMode;

/// Main editor application
class Editor : public Pina::Application {
public:
    Editor();
    ~Editor() override;

protected:
    void onInit() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onRenderUI() override;
    void onResize(int width, int height) override;
    void onShutdown() override;

private:
    void setupDefaultScene();
    void renderMenuBar();
    void renderToolbar();

    // Graphics
    Pina::UNIQUE<Pina::GraphicsDevice> m_device;

    // Scene
    std::unique_ptr<Pina::Scene> m_scene;
    Pina::UNIQUE<Pina::Shader> m_shader;
    Pina::DirectionalLight m_sunLight;

    // Editor components
    std::unique_ptr<Selection> m_selection;
    std::unique_ptr<EditorCamera> m_editorCamera;

    // Panels
    std::unique_ptr<HierarchyPanel> m_hierarchyPanel;
    std::unique_ptr<InspectorPanel> m_inspectorPanel;
    std::unique_ptr<ViewportPanel> m_viewportPanel;

    // Gizmo settings
    GizmoMode m_gizmoMode;
};

} // namespace PinaEditor
