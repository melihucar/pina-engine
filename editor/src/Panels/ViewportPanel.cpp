#include "ViewportPanel.h"
#include "../Selection.h"
#include "../EditorCamera.h"
#include "../Gizmos/GizmoRenderer.h"
#include "../Gizmos/Gizmo.h"
#include "../Gizmos/TranslateGizmo.h"
#include "../Gizmos/RotateGizmo.h"
#include "../Gizmos/ScaleGizmo.h"
#include <Pina.h>
#include <imgui.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace PinaEditor {

ViewportPanel::ViewportPanel(Pina::Scene* scene, Selection* selection, EditorCamera* camera)
    : Panel("Viewport")
    , m_scene(scene)
    , m_selection(selection)
    , m_editorCamera(camera)
    , m_gizmoMode(GizmoMode::Translate)
{
    m_translateGizmo = std::make_unique<TranslateGizmo>();
    m_rotateGizmo = std::make_unique<RotateGizmo>();
    m_scaleGizmo = std::make_unique<ScaleGizmo>();
}

ViewportPanel::~ViewportPanel() {
    deleteFramebuffer();
}

void ViewportPanel::setGraphicsDevice(Pina::GraphicsDevice* device) {
    m_graphicsDevice = device;
    m_sceneRenderer = std::make_unique<Pina::SceneRenderer>(device);
    m_gizmoRenderer = std::make_unique<GizmoRenderer>(device);
}

void ViewportPanel::createFramebuffer(int width, int height) {
    if (width <= 0 || height <= 0) return;

    // Delete existing framebuffer if size changed
    if (m_framebuffer != 0 && (m_framebufferSize.x != width || m_framebufferSize.y != height)) {
        deleteFramebuffer();
    }

    if (m_framebuffer != 0) return; // Already created with correct size

    m_framebufferSize = glm::vec2(width, height);

    // Create framebuffer
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    // Create color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewportPanel::deleteFramebuffer() {
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
    if (m_colorTexture != 0) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    if (m_depthRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &m_depthRenderbuffer);
        m_depthRenderbuffer = 0;
    }
    m_framebufferSize = glm::vec2(0, 0);
}

void ViewportPanel::onRender() {
    if (!m_open) return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    Pina::Widgets::Window window("Viewport", &m_open);
    if (window) {
        m_viewportFocused = ImGui::IsWindowFocused();
        m_viewportHovered = ImGui::IsWindowHovered();

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        m_viewportSize = glm::vec2(viewportSize.x, viewportSize.y);

        // Ensure minimum size
        if (viewportSize.x < 1) viewportSize.x = 1;
        if (viewportSize.y < 1) viewportSize.y = 1;

        // Create/resize framebuffer if needed
        createFramebuffer(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));

        // Update camera aspect ratio
        if (m_editorCamera && viewportSize.x > 0 && viewportSize.y > 0) {
            Pina::Camera* cam = m_editorCamera->getCamera();
            if (cam) {
                cam->setPerspective(45.0f, viewportSize.x / viewportSize.y, 0.1f, 1000.0f);
            }
        }

        // Render to framebuffer
        if (m_framebuffer != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
            glViewport(0, 0, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));

            // Render scene
            renderScene();

            // Render gizmos
            renderGizmos();

            // Unbind framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Display the framebuffer texture in ImGui
            // Flip the texture vertically by using UV coords (0,1) to (1,0)
            ImGui::Image(
                (ImTextureID)(intptr_t)m_colorTexture,
                viewportSize,
                ImVec2(0, 1), ImVec2(1, 0)
            );

            // Shading mode overlay UI (top-right corner of viewport)
            ImVec2 overlayPos = ImGui::GetWindowPos();
            overlayPos.x += viewportSize.x - 120.0f;
            overlayPos.y += ImGui::GetWindowContentRegionMin().y + 8.0f;
            ImGui::SetNextWindowPos(overlayPos);
            ImGui::SetNextWindowBgAlpha(0.7f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 6));
            if (ImGui::Begin("##ShadingMode", nullptr,
                             ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
                ImGui::Text("Shading");
                ImGui::Separator();
                int shadingModeInt = static_cast<int>(m_shadingMode);
                if (ImGui::RadioButton("Smooth", &shadingModeInt, 0)) {
                    m_shadingMode = ShadingMode::Smooth;
                }
                if (ImGui::RadioButton("Flat", &shadingModeInt, 1)) {
                    m_shadingMode = ShadingMode::Flat;
                }
                if (ImGui::RadioButton("Wire", &shadingModeInt, 2)) {
                    m_shadingMode = ShadingMode::Wireframe;
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
        }

        // Gizmo mode shortcuts (when viewport is focused)
        if (m_viewportFocused && m_input) {
            if (m_input->isKeyPressed(Pina::Key::W)) {
                m_gizmoMode = GizmoMode::Translate;
            }
            if (m_input->isKeyPressed(Pina::Key::E)) {
                m_gizmoMode = GizmoMode::Rotate;
            }
            if (m_input->isKeyPressed(Pina::Key::R)) {
                m_gizmoMode = GizmoMode::Scale;
            }
        }
    }

    ImGui::PopStyleVar();
}

void ViewportPanel::onUpdate(float deltaTime) {
    // Update editor camera if viewport is focused
    if (m_viewportFocused && m_editorCamera && m_input) {
        m_editorCamera->update(m_input, deltaTime);
    }
}

void ViewportPanel::renderScene() {
    if (!m_scene || !m_shader || !m_sceneRenderer || !m_editorCamera || !m_graphicsDevice) return;

    // Set the editor camera as active for rendering
    Pina::Camera* camera = m_editorCamera->getCamera();
    if (!camera) return;

    // Clear framebuffer using graphics device abstraction
    m_graphicsDevice->clear(0.15f, 0.15f, 0.15f, 1.0f);
    m_graphicsDevice->setDepthTest(true);

    // Apply shading mode using graphics device abstraction
    bool wireframeMode = (m_shadingMode == ShadingMode::Wireframe);
    m_graphicsDevice->setWireframe(wireframeMode);

    // Render scene with editor camera
    m_shader->bind();
    m_shader->setMat4("u_view", camera->getViewMatrix());
    m_shader->setMat4("u_projection", camera->getProjectionMatrix());
    m_shader->setVec3("u_viewPos", camera->getPosition());

    // Set shading mode uniform for shader (0 = smooth, 1 = flat, 2 = wireframe)
    m_shader->setInt("u_shadingMode", static_cast<int>(m_shadingMode));

    // Upload lights from scene
    Pina::LightManager& lightManager = m_scene->getLightManager();
    lightManager.uploadToShader(m_shader);

    // Render all nodes
    Pina::Node* root = m_scene->getRoot();
    if (root) {
        // Use SceneRenderer to traverse and render
        m_sceneRenderer->render(m_scene, m_shader);
    }

    // Reset wireframe mode after rendering
    if (wireframeMode) {
        m_graphicsDevice->setWireframe(false);
    }
}

void ViewportPanel::renderGizmos() {
    if (!m_gizmoRenderer || !m_editorCamera) return;

    Pina::Camera* camera = m_editorCamera->getCamera();
    if (!camera) return;

    m_gizmoRenderer->begin();

    // Draw a ground grid for visual reference
    const float gridSize = 10.0f;
    const int gridLines = 21;
    const float gridStep = gridSize / (gridLines - 1);
    const float halfGrid = gridSize / 2.0f;
    glm::vec4 gridColor(0.4f, 0.4f, 0.4f, 1.0f);

    for (int i = 0; i < gridLines; ++i) {
        float pos = -halfGrid + i * gridStep;
        // Z-axis lines
        m_gizmoRenderer->drawLine(glm::vec3(pos, 0, -halfGrid), glm::vec3(pos, 0, halfGrid), gridColor);
        // X-axis lines
        m_gizmoRenderer->drawLine(glm::vec3(-halfGrid, 0, pos), glm::vec3(halfGrid, 0, pos), gridColor);
    }

    // Draw wireframe boxes for all nodes (to make them visible)
    if (m_scene) {
        m_scene->traverse([this](Pina::Node* node) {
            if (node && node->isEnabled() && !node->hasModel()) {
                // Use full world matrix so rotation and scale are reflected
                const glm::mat4& worldMatrix = node->getTransform().getWorldMatrix();
                // Draw a small wireframe cube with full transform
                bool isSelected = m_selection && m_selection->getSelected() == node;
                glm::vec4 color = isSelected ? glm::vec4(1.0f, 0.8f, 0.0f, 1.0f) : glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
                m_gizmoRenderer->drawCubeTransformed(worldMatrix, 0.3f, color);
            }
        });
    }

    // Draw gizmo for selected object
    if (m_selection && m_selection->hasSelection()) {
        Pina::Node* selected = m_selection->getSelected();
        if (selected) {
            // Get the appropriate gizmo
            Gizmo* activeGizmo = nullptr;
            switch (m_gizmoMode) {
                case GizmoMode::Translate:
                    activeGizmo = m_translateGizmo.get();
                    break;
                case GizmoMode::Rotate:
                    activeGizmo = m_rotateGizmo.get();
                    break;
                case GizmoMode::Scale:
                    activeGizmo = m_scaleGizmo.get();
                    break;
            }

            if (activeGizmo) {
                // Set viewport bounds for proper coordinate conversion
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
                glm::vec2 viewportPos(windowPos.x + contentMin.x, windowPos.y + contentMin.y);
                activeGizmo->setViewportBounds(viewportPos, m_viewportSize);

                // Handle gizmo input
                if (m_input && m_viewportHovered) {
                    activeGizmo->handleInput(m_input, camera, &selected->getTransform());
                }

                // Render gizmo
                activeGizmo->render(m_gizmoRenderer.get(), selected->getTransform().getWorldMatrix(), camera);
            }
        }
    }

    m_gizmoRenderer->flush(camera);
}

GizmoMode ViewportPanel::getGizmoMode() const {
    return m_gizmoMode;
}

void ViewportPanel::setGizmoMode(GizmoMode mode) {
    m_gizmoMode = mode;
}

} // namespace PinaEditor
