/// Pina Engine - ImGui UI Implementation (macOS)
/// Uses imgui_impl_osx.mm and imgui_impl_opengl3.cpp backends

#include "ImGuiUI.h"
#include "ImGuiTheme.h"
#include "../../Platform/Window.h"
#include "../../Platform/Graphics.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_osx.h>

#import <Cocoa/Cocoa.h>

namespace Pina {

ImGuiUI::ImGuiUI() = default;

ImGuiUI::~ImGuiUI() {
    destroy();
}

bool ImGuiUI::create(Window* window, Graphics* graphics, const UIConfig& config) {
    if (m_initialized) {
        return true;
    }

    m_window = window;
    m_graphics = graphics;

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    // Enable docking
    if (config.enableDocking) {
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    // Enable multi-viewport (experimental)
    if (config.enableViewports) {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }

    // Load Pina Engine font (Roboto)
    loadPinaFont(config.fontSize);

    // Apply Pina Engine theme (colors, rounding, spacing)
    applyPinaTheme();

    // When viewports are enabled, ensure opaque window background
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Initialize platform backend (macOS)
    NSView* view = (__bridge NSView*)window->getNativeView();
    ImGui_ImplOSX_Init(view);

    // Initialize renderer backend (OpenGL 3)
    ImGui_ImplOpenGL3_Init("#version 150");

    m_initialized = true;
    return true;
}

void ImGuiUI::destroy() {
    if (!m_initialized) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplOSX_Shutdown();
    ImGui::DestroyContext();

    m_window = nullptr;
    m_graphics = nullptr;
    m_initialized = false;
}

void ImGuiUI::beginFrame() {
    if (!m_initialized) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplOSX_NewFrame((__bridge NSView*)m_window->getNativeView());
    ImGui::NewFrame();
}

void ImGuiUI::endFrame() {
    if (!m_initialized) {
        return;
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and render additional platform windows (multi-viewport)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        // Restore our OpenGL context
        m_graphics->makeCurrent();
    }
}

bool ImGuiUI::wantsCaptureKeyboard() const {
    if (!m_initialized) {
        return false;
    }
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool ImGuiUI::wantsCaptureMouse() const {
    if (!m_initialized) {
        return false;
    }
    return ImGui::GetIO().WantCaptureMouse;
}

void ImGuiUI::showDemoWindow(bool* open) {
    if (!m_initialized) {
        return;
    }
    ImGui::ShowDemoWindow(open);
}

void ImGuiUI::initialize() {
    // Called after all subsystems are registered
    // UI is already created in Application::run() before this
}

void ImGuiUI::update(float deltaTime) {
    (void)deltaTime;
    // UI updates happen in beginFrame/endFrame
}

void ImGuiUI::shutdown() {
    destroy();
}

// Factory implementation
UISubsystem* UISubsystem::createDefault() {
    return new ImGuiUI();
}

} // namespace Pina
