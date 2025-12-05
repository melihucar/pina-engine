#pragma once

#include <string>

namespace PinaEditor {

/// Base class for editor panels
class Panel {
public:
    explicit Panel(const std::string& name) : m_name(name) {}
    virtual ~Panel() = default;

    // Render the panel (must be called within ImGui context)
    virtual void onRender() = 0;

    // Update logic (called every frame)
    virtual void onUpdate(float deltaTime) { (void)deltaTime; }

    // Panel state
    bool isOpen() const { return m_open; }
    void setOpen(bool open) { m_open = open; }
    void toggleOpen() { m_open = !m_open; }

    const std::string& getName() const { return m_name; }

protected:
    std::string m_name;
    bool m_open = true;
};

} // namespace PinaEditor
