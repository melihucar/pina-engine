#pragma once

/// Pina Engine - UI Theme
/// Cross-platform theme and font loading for ImGui

namespace Pina {

/// Apply the Pina Engine UI theme (colors, rounding, spacing)
/// Must be called after ImGui::CreateContext()
void applyPinaTheme();

/// Load the Pina Engine default font (Roboto)
/// Must be called after ImGui::CreateContext() and before backend initialization
/// @param fontSize Base font size in pixels (default 15.0f)
void loadPinaFont(float fontSize = 15.0f);

} // namespace Pina
