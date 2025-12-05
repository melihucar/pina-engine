#pragma once

#include <functional>

namespace Pina {
    class Node;
}

namespace PinaEditor {

/// Selection state management for the editor
class Selection {
public:
    using SelectionCallback = std::function<void(Pina::Node*)>;

    Selection() = default;
    ~Selection() = default;

    // Selection operations
    void select(Pina::Node* node);
    void deselect();

    // Query
    Pina::Node* getSelected() const { return m_selected; }
    bool hasSelection() const { return m_selected != nullptr; }

    // Callback for selection changes
    void setOnSelectionChanged(SelectionCallback callback) { m_onSelectionChanged = std::move(callback); }

private:
    Pina::Node* m_selected = nullptr;
    SelectionCallback m_onSelectionChanged;
};

} // namespace PinaEditor
