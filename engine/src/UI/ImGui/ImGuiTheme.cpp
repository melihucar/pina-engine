/// Pina Engine - UI Theme Implementation
/// Cross-platform theme and font loading for ImGui

#include "ImGuiTheme.h"
#include <imgui.h>

namespace Pina {

void applyPinaTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // === Geometry (Modern/Rounded) ===
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    // === Colors (VS Code inspired dark theme with blue accent) ===

    // Window backgrounds
    colors[ImGuiCol_WindowBg]           = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Border]             = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_BorderShadow]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame backgrounds (inputs, checkboxes, etc.)
    colors[ImGuiCol_FrameBg]            = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

    // Title bar
    colors[ImGuiCol_TitleBg]            = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    // Menu bar
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Accent color (blue) - checkmarks, sliders
    colors[ImGuiCol_CheckMark]          = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.00f, 0.55f, 0.90f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.00f, 0.40f, 0.70f, 1.00f);

    // Headers (collapsing headers, tree nodes, selectables)
    colors[ImGuiCol_Header]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.00f, 0.48f, 0.80f, 0.80f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab]                = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.00f, 0.48f, 0.80f, 0.80f);
    colors[ImGuiCol_TabSelected]        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline]= ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_TabDimmed]          = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected]  = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    // Text
    colors[ImGuiCol_Text]               = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.00f, 0.48f, 0.80f, 0.35f);

    // Separators
    colors[ImGuiCol_Separator]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.00f, 0.48f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]    = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);

    // Resize grip
    colors[ImGuiCol_ResizeGrip]         = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.00f, 0.48f, 0.80f, 0.80f);
    colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);

    // Docking
    colors[ImGuiCol_DockingPreview]     = ImVec4(0.00f, 0.48f, 0.80f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    // Plot (graphs)
    colors[ImGuiCol_PlotLines]          = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]   = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_PlotHistogram]      = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.55f, 0.90f, 1.00f);

    // Tables
    colors[ImGuiCol_TableHeaderBg]      = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]  = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_TableBorderLight]   = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    colors[ImGuiCol_TableRowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]      = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

    // Navigation
    colors[ImGuiCol_NavCursor]          = ImVec4(0.00f, 0.48f, 0.80f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]  = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

    // Modal dim
    colors[ImGuiCol_ModalWindowDimBg]   = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);

    // Drag and drop
    colors[ImGuiCol_DragDropTarget]     = ImVec4(0.00f, 0.48f, 0.80f, 0.90f);
}

void loadPinaFont(float fontSize) {
    ImGuiIO& io = ImGui::GetIO();

    // Try relative to executable: fonts/Roboto-Medium.ttf
    ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", fontSize);
    if (font) {
        io.FontDefault = font;
        return;
    }

    // Fallback: use ImGui default font
    io.Fonts->AddFontDefault();
}

} // namespace Pina
