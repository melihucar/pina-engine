#pragma once

#include "Panel.h"

namespace Pina {
    class Scene;
    class Node;
}

namespace PinaEditor {

class Selection;

/// Hierarchy panel - displays scene tree structure
class HierarchyPanel : public Panel {
public:
    HierarchyPanel(Pina::Scene* scene, Selection* selection);

    void onRender() override;

    void setScene(Pina::Scene* scene) { m_scene = scene; }

private:
    void renderNode(Pina::Node* node);
    void showContextMenu(Pina::Node* node);

    Pina::Scene* m_scene = nullptr;
    Selection* m_selection = nullptr;

    // Context menu state
    Pina::Node* m_contextMenuNode = nullptr;
};

} // namespace PinaEditor
