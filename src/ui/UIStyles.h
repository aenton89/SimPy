//
// Created by tajbe on 24.10.2025.
//
#pragma once

// #include <imgui.h>



/*
 * defines common colors used in UI
 */
namespace UIStyles {
	constexpr ImVec4 SELECTION_COLOR = ImVec4(0.39f, 0.78f, 0.92f, 0.77f);

	constexpr ImU32 CONNECTION_COLOR = IM_COL32(255, 255, 0, 255);
	constexpr ImU32 CONNECTION_HOVER_COLOR = IM_COL32(255, 100, 100, 255);
	constexpr ImU32 CONNECTION_DRAGGING_COLOR = IM_COL32(255, 255, 0, 100);

	constexpr ImU32 CONTROL_NODE_NORMAL = IM_COL32(200, 200, 200, 200);
	constexpr ImU32 CONTROL_NODE_HOVER = IM_COL32(255, 150, 0, 255);
	constexpr ImU32 CONTROL_NODE_BORDER = IM_COL32(50, 50, 50, 255);

	constexpr ImU32 BUTTON_NORMAL = IM_COL32(200, 200, 0, 255);
	constexpr ImU32 BUTTON_HOVER = IM_COL32(255, 255, 0, 255);
	constexpr ImU32 BUTTON_ACTIVE = IM_COL32(255, 0, 0, 255);
	constexpr ImU32 BUTTON_PLUS_COLOR = IM_COL32(0, 0, 0, 255);

	constexpr ImVec4 DARK_BACKGROUND = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	constexpr ImVec4 LIGHT_BACKGROUND = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

	constexpr ImU32 GRID_COLOR_DARK = IM_COL32(0, 0, 0, 100);
	constexpr ImU32 GRID_COLOR_LIGHT = IM_COL32(200, 200, 200, 100);

	inline void applyDarkStyle() {
		ImGui::StyleColorsClassic();
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg] = DARK_BACKGROUND;
		style.Colors[ImGuiCol_ChildBg] = DARK_BACKGROUND;
		style.Colors[ImGuiCol_PopupBg] = DARK_BACKGROUND;
	}

	// apply light style
	inline void applyLightStyle() {
		ImGui::StyleColorsLight();
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg] = LIGHT_BACKGROUND;
		style.Colors[ImGuiCol_ChildBg] = LIGHT_BACKGROUND;
		style.Colors[ImGuiCol_PopupBg] = LIGHT_BACKGROUND;
	}
}
