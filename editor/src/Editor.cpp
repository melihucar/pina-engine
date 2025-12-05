#include "Editor.h"
#include "Selection.h"
#include "EditorCamera.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"
#include "Gizmos/Gizmo.h"
#include <imgui.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace PinaEditor {

Editor::Editor()
    : m_gizmoMode(GizmoMode::Translate)
{
    m_config.title = "Pina Editor";
    m_config.windowWidth = 1600;
    m_config.windowHeight = 900;
    m_config.vsync = true;
    m_config.resizable = true;
}

Editor::~Editor() = default;

void Editor::onInit() {
    // Create graphics device
    m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);
    m_device->setDepthTest(true);

    // Create scene
    m_scene = std::make_unique<Pina::Scene>();

    // Create shader
    m_shader = m_device->createShader();
    if (!m_shader->load(Pina::ShaderLibrary::getStandardVertexShader(),
                        Pina::ShaderLibrary::getStandardFragmentShader())) {
        std::cerr << "Failed to compile editor shader!" << std::endl;
    }

    // Create editor components
    m_selection = std::make_unique<Selection>();
    m_editorCamera = std::make_unique<EditorCamera>();

    // Create panels
    m_hierarchyPanel = std::make_unique<HierarchyPanel>(m_scene.get(), m_selection.get());
    m_inspectorPanel = std::make_unique<InspectorPanel>(m_selection.get());
    m_viewportPanel = std::make_unique<ViewportPanel>(m_scene.get(), m_selection.get(), m_editorCamera.get());

    // Configure viewport panel
    m_viewportPanel->setShader(m_shader.get());
    m_viewportPanel->setInput(getInput());
    m_viewportPanel->setGraphicsDevice(m_device.get());

    // Setup default scene
    setupDefaultScene();
}

void Editor::setupDefaultScene() {
    // Add a default directional light
    m_sunLight.setDirection(Pina::Vector3(-0.5f, -1.0f, -0.5f));
    m_sunLight.setColor(Pina::Color(1.0f, 1.0f, 1.0f));
    m_sunLight.setIntensity(1.0f);
    m_scene->getLightManager().addLight(&m_sunLight);

    // Create some default nodes
    Pina::Node* cube = m_scene->createNode("Cube");
    cube->getTransform().setLocalPosition(0.0f, 0.0f, 0.0f);

    Pina::Node* light = m_scene->createNode("Light");
    light->getTransform().setLocalPosition(2.0f, 3.0f, 2.0f);

    // Select the first node by default
    m_selection->select(cube);
}

void Editor::onUpdate(float deltaTime) {
    // Update scene
    m_scene->update(deltaTime);

    // Update viewport (camera)
    m_viewportPanel->onUpdate(deltaTime);
}

void Editor::onRender() {
    m_device->beginFrame();
    m_device->clear(0.1f, 0.1f, 0.1f);

    // Scene rendering is handled by ViewportPanel
}

void Editor::onRenderUI() {
    // Enable docking
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    (void)dockspace_id;

    // Menu bar
    renderMenuBar();

    // Toolbar
    renderToolbar();

    // Render panels
    m_hierarchyPanel->onRender();
    m_inspectorPanel->onRender();
    m_viewportPanel->onRender();

    m_device->endFrame();
}

void Editor::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                // TODO: Create new scene
            }
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                // TODO: Open scene
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                // TODO: Save scene
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                quit();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                // TODO: Undo
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                // TODO: Redo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del")) {
                if (m_selection->hasSelection()) {
                    Pina::Node* selected = m_selection->getSelected();
                    m_selection->deselect();
                    if (selected && selected->getParent()) {
                        selected->getParent()->removeChild(selected);
                    }
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Empty Node")) {
                m_scene->createNode("New Node");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            bool hierarchyOpen = m_hierarchyPanel->isOpen();
            if (ImGui::MenuItem("Hierarchy", nullptr, &hierarchyOpen)) {
                m_hierarchyPanel->setOpen(hierarchyOpen);
            }

            bool inspectorOpen = m_inspectorPanel->isOpen();
            if (ImGui::MenuItem("Inspector", nullptr, &inspectorOpen)) {
                m_inspectorPanel->setOpen(inspectorOpen);
            }

            bool viewportOpen = m_viewportPanel->isOpen();
            if (ImGui::MenuItem("Viewport", nullptr, &viewportOpen)) {
                m_viewportPanel->setOpen(viewportOpen);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Editor::renderToolbar() {
    ImGui::Begin("##Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    GizmoMode currentMode = m_viewportPanel->getGizmoMode();

    // Translate button
    bool translateActive = (currentMode == GizmoMode::Translate);
    if (translateActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
    }
    if (ImGui::Button("W: Move")) {
        m_viewportPanel->setGizmoMode(GizmoMode::Translate);
    }
    if (translateActive) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // Rotate button
    bool rotateActive = (currentMode == GizmoMode::Rotate);
    if (rotateActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
    }
    if (ImGui::Button("E: Rotate")) {
        m_viewportPanel->setGizmoMode(GizmoMode::Rotate);
    }
    if (rotateActive) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // Scale button
    bool scaleActive = (currentMode == GizmoMode::Scale);
    if (scaleActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
    }
    if (ImGui::Button("R: Scale")) {
        m_viewportPanel->setGizmoMode(GizmoMode::Scale);
    }
    if (scaleActive) {
        ImGui::PopStyleColor();
    }

    ImGui::End();
}

void Editor::onResize(int width, int height) {
    m_device->setViewport(0, 0, width, height);
    if (m_editorCamera && m_editorCamera->getCamera()) {
        m_editorCamera->getCamera()->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }
}

void Editor::onShutdown() {
    m_viewportPanel.reset();
    m_inspectorPanel.reset();
    m_hierarchyPanel.reset();
    m_editorCamera.reset();
    m_selection.reset();
    m_scene.reset();
    m_shader.reset();
    m_device.reset();
}

} // namespace PinaEditor
