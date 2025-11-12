//
// Created by tajbe on 07.11.2025.
//
#pragma once

#include <imgui.h>
#include <cereal/archives/xml.hpp>
#include "../ui/UIStyles.h"

class GUICore;



/*
 * manages viewport operations (zooming, panning), and grid operations
 */
class ViewportManager {
private:
	class GUICore* guiCore = nullptr;

public:
	float zoomAmount = 1.0f;
	ImVec2 viewOffset = ImVec2(0.0, 0.0);
	float zoomSpeed = 0.1f;
	// przesuwanie canvas'u
	bool canvasDragging = false;
	ImVec2 dragStartPos = ImVec2(0.0, 0.0);
	ImVec2 dragStartOffset = ImVec2(0.0, 0.0);

	void setGUICore(GUICore* gui);

	void zoomAndPanning();
	void drawGrid();
	void lightMode();
	// skróty klawiszowe
	void turnLightModeOnShortcut(const ImGuiIO& io);
	void turnGridOnShortcut(const ImGuiIO& io);

	template<class Archive>
	void serialize(Archive& ar) {
		ar(CEREAL_NVP(zoomAmount),
		   CEREAL_NVP(zoomSpeed));

		std::vector<float> offset = {viewOffset.x, viewOffset.y};
		ar(cereal::make_nvp("viewOffset", offset));

		// reset runtime state
		if constexpr (Archive::is_loading::value) {
			viewOffset = ImVec2(offset[0], offset[1]);
		}
	}
};
