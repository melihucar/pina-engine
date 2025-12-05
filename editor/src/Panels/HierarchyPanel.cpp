#include "HierarchyPanel.h"
#include "../Selection.h"
#include <Pina.h>
#include <imgui.h>

namespace PinaEditor {

HierarchyPanel::HierarchyPanel(Pina::Scene* scene, Selection* selection)
    : Panel("Hierarchy")
    , m_scene(scene)
    , m_selection(selection)
{
}

void HierarchyPanel::onRender() {
    if (!m_open) return;

    Pina::Widgets::Window window("Hierarchy", &m_open);
    if (window) {
        if (!m_scene) {
            Pina::Widgets::Text("No scene loaded");
            return;
        }

        // Render the scene tree starting from root
        Pina::Node* root = m_scene->getRoot();
        if (root) {
            // Render children of root (root itself is typically not shown)
            for (size_t i = 0; i < root->getChildCount(); ++i) {
                renderNode(root->getChild(i));
            }
        }

        // Context menu for empty space (create new node)
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("Create Empty")) {
                m_scene->createNode("New Node");
            }
            ImGui::EndPopup();
        }
    }
}

void HierarchyPanel::renderNode(Pina::Node* node) {
    if (!node) return;

    bool isSelected = m_selection && m_selection->getSelected() == node;
    bool hasChildren = node->getChildCount() > 0;

    // Build flags
    Pina::UITreeNodeFlags flags = Pina::UITreeNodeFlags::OpenOnArrow;
    if (isSelected) {
        flags = flags | Pina::UITreeNodeFlags::Selected;
    }
    if (!hasChildren) {
        flags = flags | Pina::UITreeNodeFlags::Leaf;
    }

    // Create unique ID for ImGui
    ImGui::PushID(static_cast<int>(node->getID()));

    Pina::Widgets::TreeNode treeNode(node->getName().c_str(), flags);

    // Handle selection on click
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        if (m_selection) {
            m_selection->select(node);
        }
    }

    // Context menu
    if (ImGui::BeginPopupContextItem()) {
        showContextMenu(node);
        ImGui::EndPopup();
    }

    // Render children if node is open
    if (treeNode) {
        for (size_t i = 0; i < node->getChildCount(); ++i) {
            renderNode(node->getChild(i));
        }
    }

    ImGui::PopID();
}

void HierarchyPanel::showContextMenu(Pina::Node* node) {
    if (!node || !m_scene) return;

    if (ImGui::MenuItem("Add Child")) {
        node->addChild("New Child");
    }

    if (ImGui::MenuItem("Duplicate")) {
        // TODO: Implement node duplication
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Delete")) {
        // Deselect if this was selected
        if (m_selection && m_selection->getSelected() == node) {
            m_selection->deselect();
        }

        // Remove from parent
        Pina::Node* parent = node->getParent();
        if (parent) {
            parent->removeChild(node);
        }
    }

    ImGui::Separator();

    // Enable/disable toggle
    bool enabled = node->isEnabled();
    if (ImGui::MenuItem(enabled ? "Disable" : "Enable")) {
        node->setEnabled(!enabled);
    }
}

} // namespace PinaEditor
