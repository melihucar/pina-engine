#pragma once

#include "Panel.h"
#include <glm/glm.hpp>

namespace Pina {
    class Node;
    class Material;
}

namespace PinaEditor {

class Selection;

/// Inspector panel - displays and edits properties of selected object
class InspectorPanel : public Panel {
public:
    explicit InspectorPanel(Selection* selection);

    void onRender() override;

private:
    void renderNodeProperties(Pina::Node* node);
    void renderTransform(Pina::Node* node);
    void renderMaterial(Pina::Material* material);

    // Helper to render a vec3 edit control
    bool renderVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f);

    Selection* m_selection = nullptr;
};

} // namespace PinaEditor
