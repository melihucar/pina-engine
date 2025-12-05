/// Pina Engine - ImGui Widget Wrappers
/// Implements UIWidgets.h using Dear ImGui

#include "../UIWidgets.h"
#include <imgui.h>

namespace Pina::Widgets {

// ============================================================================
// Helper: Convert Pina flags to ImGui flags
// ============================================================================

namespace {

// Bring in Pina types and operators
using namespace Pina;

ImGuiWindowFlags toImGuiWindowFlags(UIWindowFlags flags) {
    ImGuiWindowFlags result = 0;
    if ((flags & UIWindowFlags::NoTitleBar) != UIWindowFlags::None) result |= ImGuiWindowFlags_NoTitleBar;
    if ((flags & UIWindowFlags::NoResize) != UIWindowFlags::None) result |= ImGuiWindowFlags_NoResize;
    if ((flags & UIWindowFlags::NoMove) != UIWindowFlags::None) result |= ImGuiWindowFlags_NoMove;
    if ((flags & UIWindowFlags::NoScrollbar) != UIWindowFlags::None) result |= ImGuiWindowFlags_NoScrollbar;
    if ((flags & UIWindowFlags::NoCollapse) != UIWindowFlags::None) result |= ImGuiWindowFlags_NoCollapse;
    if ((flags & UIWindowFlags::AlwaysAutoResize) != UIWindowFlags::None) result |= ImGuiWindowFlags_AlwaysAutoResize;
    if ((flags & UIWindowFlags::NoBackground) != UIWindowFlags::None) result |= ImGuiWindowFlags_NoBackground;
    if ((flags & UIWindowFlags::NoSavedSettings) != UIWindowFlags::None) result |= ImGuiWindowFlags_NoSavedSettings;
    if ((flags & UIWindowFlags::MenuBar) != UIWindowFlags::None) result |= ImGuiWindowFlags_MenuBar;
    if ((flags & UIWindowFlags::HorizontalScrollbar) != UIWindowFlags::None) result |= ImGuiWindowFlags_HorizontalScrollbar;
    return result;
}

ImGuiInputTextFlags toImGuiInputTextFlags(UIInputTextFlags flags) {
    ImGuiInputTextFlags result = 0;
    if ((flags & UIInputTextFlags::CharsDecimal) != UIInputTextFlags::None) result |= ImGuiInputTextFlags_CharsDecimal;
    if ((flags & UIInputTextFlags::CharsHexadecimal) != UIInputTextFlags::None) result |= ImGuiInputTextFlags_CharsHexadecimal;
    if ((flags & UIInputTextFlags::CharsUppercase) != UIInputTextFlags::None) result |= ImGuiInputTextFlags_CharsUppercase;
    if ((flags & UIInputTextFlags::CharsNoBlank) != UIInputTextFlags::None) result |= ImGuiInputTextFlags_CharsNoBlank;
    if ((flags & UIInputTextFlags::AutoSelectAll) != UIInputTextFlags::None) result |= ImGuiInputTextFlags_AutoSelectAll;
    if ((flags & UIInputTextFlags::EnterReturnsTrue) != UIInputTextFlags::None) result |= ImGuiInputTextFlags_EnterReturnsTrue;
    if ((flags & UIInputTextFlags::Password) != UIInputTextFlags::None) result |= ImGuiInputTextFlags_Password;
    if ((flags & UIInputTextFlags::ReadOnly) != UIInputTextFlags::None) result |= ImGuiInputTextFlags_ReadOnly;
    return result;
}

ImGuiTreeNodeFlags toImGuiTreeNodeFlags(UITreeNodeFlags flags) {
    ImGuiTreeNodeFlags result = 0;
    if ((flags & UITreeNodeFlags::Selected) != UITreeNodeFlags::None) result |= ImGuiTreeNodeFlags_Selected;
    if ((flags & UITreeNodeFlags::Framed) != UITreeNodeFlags::None) result |= ImGuiTreeNodeFlags_Framed;
    if ((flags & UITreeNodeFlags::DefaultOpen) != UITreeNodeFlags::None) result |= ImGuiTreeNodeFlags_DefaultOpen;
    if ((flags & UITreeNodeFlags::OpenOnArrow) != UITreeNodeFlags::None) result |= ImGuiTreeNodeFlags_OpenOnArrow;
    if ((flags & UITreeNodeFlags::Leaf) != UITreeNodeFlags::None) result |= ImGuiTreeNodeFlags_Leaf;
    if ((flags & UITreeNodeFlags::Bullet) != UITreeNodeFlags::None) result |= ImGuiTreeNodeFlags_Bullet;
    return result;
}

ImGuiSelectableFlags toImGuiSelectableFlags(UISelectableFlags flags) {
    ImGuiSelectableFlags result = 0;
    if ((flags & UISelectableFlags::DontClosePopups) != UISelectableFlags::None) result |= ImGuiSelectableFlags_DontClosePopups;
    if ((flags & UISelectableFlags::SpanAllColumns) != UISelectableFlags::None) result |= ImGuiSelectableFlags_SpanAllColumns;
    if ((flags & UISelectableFlags::AllowDoubleClick) != UISelectableFlags::None) result |= ImGuiSelectableFlags_AllowDoubleClick;
    return result;
}

ImVec2 toImVec2(const Vector2& v) {
    return ImVec2(v.x, v.y);
}

ImVec4 toImVec4(const Color& c) {
    return ImVec4(c.r, c.g, c.b, c.a);
}

} // anonymous namespace

// ============================================================================
// Container Widgets
// ============================================================================

Window::Window(const char* title, bool* open, UIWindowFlags flags) {
    m_visible = ImGui::Begin(title, open, toImGuiWindowFlags(flags));
}

Window::~Window() {
    ImGui::End();
}

Child::Child(const char* id, const Vector2& size, bool border) {
    m_visible = ImGui::BeginChild(id, toImVec2(size), border ? ImGuiChildFlags_Borders : ImGuiChildFlags_None);
}

Child::~Child() {
    ImGui::EndChild();
}

TreeNode::TreeNode(const char* label, UITreeNodeFlags flags) {
    m_open = ImGui::TreeNodeEx(label, toImGuiTreeNodeFlags(flags));
}

TreeNode::~TreeNode() {
    if (m_open) {
        ImGui::TreePop();
    }
}

MenuBar::MenuBar() {
    m_visible = ImGui::BeginMenuBar();
}

MenuBar::~MenuBar() {
    if (m_visible) {
        ImGui::EndMenuBar();
    }
}

Menu::Menu(const char* label) {
    m_open = ImGui::BeginMenu(label);
}

Menu::~Menu() {
    if (m_open) {
        ImGui::EndMenu();
    }
}

// ============================================================================
// Display Widgets
// ============================================================================

Text::Text(const char* text) {
    ImGui::TextUnformatted(text);
}

Text::Text(const Color& color, const char* text) {
    ImGui::TextColored(toImVec4(color), "%s", text);
}

Separator::Separator() {
    ImGui::Separator();
}

SameLine::SameLine(float offsetX, float spacing) {
    ImGui::SameLine(offsetX, spacing);
}

Spacing::Spacing() {
    ImGui::Spacing();
}

Indent::Indent(float width) : m_width(width) {
    ImGui::Indent(width);
}

Indent::~Indent() {
    ImGui::Unindent(m_width);
}

// ============================================================================
// Interactive Widgets
// ============================================================================

Button::Button(const char* label, const Vector2& size) {
    m_clicked = ImGui::Button(label, toImVec2(size));
}

SmallButton::SmallButton(const char* label) {
    m_clicked = ImGui::SmallButton(label);
}

Checkbox::Checkbox(const char* label, bool* value) {
    m_changed = ImGui::Checkbox(label, value);
}

SliderFloat::SliderFloat(const char* label, float* value, float min, float max, const char* format) {
    m_changed = ImGui::SliderFloat(label, value, min, max, format);
}

SliderInt::SliderInt(const char* label, int* value, int min, int max) {
    m_changed = ImGui::SliderInt(label, value, min, max);
}

DragFloat::DragFloat(const char* label, float* value, float speed, float min, float max) {
    m_changed = ImGui::DragFloat(label, value, speed, min, max);
}

InputFloat::InputFloat(const char* label, float* value, float step) {
    m_changed = ImGui::InputFloat(label, value, step);
}

InputInt::InputInt(const char* label, int* value) {
    m_changed = ImGui::InputInt(label, value);
}

InputText::InputText(const char* label, char* buf, size_t bufSize, UIInputTextFlags flags) {
    m_changed = ImGui::InputText(label, buf, bufSize, toImGuiInputTextFlags(flags));
}

CollapsingHeader::CollapsingHeader(const char* label, UITreeNodeFlags flags) {
    m_open = ImGui::CollapsingHeader(label, toImGuiTreeNodeFlags(flags));
}

Selectable::Selectable(const char* label, bool selected, UISelectableFlags flags, const Vector2& size) {
    m_clicked = ImGui::Selectable(label, selected, toImGuiSelectableFlags(flags), toImVec2(size));
}

MenuItem::MenuItem(const char* label, const char* shortcut, bool selected, bool enabled) {
    m_clicked = ImGui::MenuItem(label, shortcut, selected, enabled);
}

ColorEdit::ColorEdit(const char* label, Color* color) {
    float col[4] = { color->r, color->g, color->b, color->a };
    m_changed = ImGui::ColorEdit4(label, col);
    if (m_changed) {
        color->r = col[0];
        color->g = col[1];
        color->b = col[2];
        color->a = col[3];
    }
}

Combo::Combo(const char* label, int* currentItem, const char* items) {
    m_changed = ImGui::Combo(label, currentItem, items);
}

// ============================================================================
// Utility Functions
// ============================================================================

bool isItemHovered() {
    return ImGui::IsItemHovered();
}

bool isItemClicked() {
    return ImGui::IsItemClicked();
}

bool isItemActive() {
    return ImGui::IsItemActive();
}

Vector2 getContentRegionAvail() {
    ImVec2 v = ImGui::GetContentRegionAvail();
    return Vector2(v.x, v.y);
}

Vector2 getWindowSize() {
    ImVec2 v = ImGui::GetWindowSize();
    return Vector2(v.x, v.y);
}

void setNextWindowPos(const Vector2& pos) {
    ImGui::SetNextWindowPos(toImVec2(pos));
}

void setNextWindowSize(const Vector2& size) {
    ImGui::SetNextWindowSize(toImVec2(size));
}

} // namespace Pina::Widgets
