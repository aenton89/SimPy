//
// Created by tajbe on 07.11.2025.
//
#pragma once

#include "managers/BasicManager.h"
// #include <imgui.h>
// #include <cereal/types/vector.hpp>
// #include <cereal/archives/xml.hpp>



/*
 * manages viewport operations (zooming, panning), and grid operations
 */
class ViewportManager : public BasicManager {
public:
	float zoomAmount = 1.0f;
	ImVec2 viewOffset = ImVec2(0.0, 0.0);
	float zoomSpeed = 0.1f;
	// przesuwanie canvas'u
	bool canvasDragging = false;
	ImVec2 dragStartPos = ImVec2(0.0, 0.0);
	ImVec2 dragStartOffset = ImVec2(0.0, 0.0);

	void zoomAndPanning();
	void drawGrid() const;
	void lightMode() const;
	// skróty klawiszowe
	void turnLightModeOnShortcut(const ImGuiIO& io) const;
	void turnGridOnShortcut(const ImGuiIO& io) const;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(cereal::base_class<BasicManager>(this));

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
