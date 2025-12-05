#include "InspectorPanel.h"
#include "../Selection.h"
#include <Pina.h>
#include <imgui.h>

namespace PinaEditor {

InspectorPanel::InspectorPanel(Selection* selection)
    : Panel("Inspector")
    , m_selection(selection)
{
}

void InspectorPanel::onRender() {
    if (!m_open) return;

    Pina::Widgets::Window window("Inspector", &m_open);
    if (window) {
        if (!m_selection || !m_selection->hasSelection()) {
            Pina::Widgets::Text("No object selected");
            return;
        }

        Pina::Node* node = m_selection->getSelected();
        if (node) {
            renderNodeProperties(node);
        }
    }
}

void InspectorPanel::renderNodeProperties(Pina::Node* node) {
    if (!node) return;

    // Node name
    char nameBuffer[256];
    strncpy(nameBuffer, node->getName().c_str(), sizeof(nameBuffer) - 1);
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';

    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        node->setName(nameBuffer);
    }

    ImGui::Separator();

    // Enabled checkbox
    bool enabled = node->isEnabled();
    if (ImGui::Checkbox("Enabled", &enabled)) {
        node->setEnabled(enabled);
    }

    ImGui::Separator();

    // Transform section
    renderTransform(node);

    ImGui::Separator();

    // Model/Material section if node has model
    if (node->hasModel()) {
        Pina::Widgets::CollapsingHeader modelHeader("Model", Pina::UITreeNodeFlags::DefaultOpen);
        if (modelHeader) {
            Pina::Model* model = node->getModel();
            ImGui::Text("Meshes: %zu", model->getMeshCount());
            // TODO: Show material properties when Material is accessible from Model
        }
    }
}

void InspectorPanel::renderTransform(Pina::Node* node) {
    Pina::Widgets::CollapsingHeader header("Transform", Pina::UITreeNodeFlags::DefaultOpen);
    if (!header) return;

    Pina::Transform& transform = node->getTransform();

    // Position
    glm::vec3 position = transform.getLocalPosition();
    if (renderVec3Control("Position", position)) {
        transform.setLocalPosition(position);
    }

    // Rotation (euler angles)
    glm::vec3 rotation = transform.getLocalRotationEuler();
    if (renderVec3Control("Rotation", rotation)) {
        transform.setLocalRotationEuler(rotation);
    }

    // Scale
    glm::vec3 scale = transform.getLocalScale();
    if (renderVec3Control("Scale", scale, 1.0f)) {
        transform.setLocalScale(scale);
    }
}

bool InspectorPanel::renderVec3Control(const char* label, glm::vec3& values, float resetValue) {
    bool modified = false;

    ImGui::PushID(label);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 80.0f);
    ImGui::Text("%s", label);
    ImGui::NextColumn();

    float itemWidth = (ImGui::CalcItemWidth() - 8.0f) / 3.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    float lineHeight = ImGui::GetFrameHeight();
    ImVec2 buttonSize(lineHeight + 3.0f, lineHeight);

    // X
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    if (ImGui::Button("X", buttonSize)) {
        values.x = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::DragFloat("##X", &values.x, 0.1f)) {
        modified = true;
    }
    ImGui::SameLine();

    // Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    if (ImGui::Button("Y", buttonSize)) {
        values.y = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::DragFloat("##Y", &values.y, 0.1f)) {
        modified = true;
    }
    ImGui::SameLine();

    // Z
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    if (ImGui::Button("Z", buttonSize)) {
        values.z = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::DragFloat("##Z", &values.z, 0.1f)) {
        modified = true;
    }

    ImGui::PopStyleVar();

    ImGui::Columns(1);

    ImGui::PopID();

    return modified;
}

void InspectorPanel::renderMaterial(Pina::Material* material) {
    if (!material) return;

    // TODO: Add material property editing when Material API is extended
    Pina::Widgets::Text("Material editing not yet implemented");
}

} // namespace PinaEditor
