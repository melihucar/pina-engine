#include "Selection.h"

namespace PinaEditor {

void Selection::select(Pina::Node* node) {
    if (m_selected != node) {
        m_selected = node;
        if (m_onSelectionChanged) {
            m_onSelectionChanged(m_selected);
        }
    }
}

void Selection::deselect() {
    select(nullptr);
}

} // namespace PinaEditor
