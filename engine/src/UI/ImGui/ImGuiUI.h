#pragma once

/// Pina Engine - ImGui UI Implementation
/// Internal implementation of the UI subsystem using Dear ImGui

#include "../UI.h"

namespace Pina {

class Window;
class Graphics;

/// ImGui-based implementation of the UI subsystem
class ImGuiUI : public UISubsystem {
public:
    ImGuiUI();
    ~ImGuiUI() override;

    // UISubsystem interface
    bool create(Window* window, Graphics* graphics, const UIConfig& config = {}) override;
    void destroy() override;
    void beginFrame() override;
    void endFrame() override;
    bool wantsCaptureKeyboard() const override;
    bool wantsCaptureMouse() const override;
    void showDemoWindow(bool* open = nullptr) override;

    // Subsystem lifecycle
    void initialize() override;
    void update(float deltaTime) override;
    void shutdown() override;

private:
    Window* m_window = nullptr;
    Graphics* m_graphics = nullptr;
    bool m_initialized = false;
};

} // namespace Pina
