//
// Created by tajbe on 10.11.2025.
//
#pragma once
#include "../../../ui/UIStyles.h"

// #include <imgui.h>
// #include <cereal/archives/xml.hpp>



struct UIPreferences {
	bool lightMode = false;

	bool gridEnabled = true;
	float gridSpacing = 50.0f;
	float gridThickness = 1.0f;
	ImU32 gridColor = UIStyles::GRID_COLOR_DARK;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(CEREAL_NVP(lightMode),
		   CEREAL_NVP(gridEnabled),
		   CEREAL_NVP(gridSpacing),
		   CEREAL_NVP(gridThickness));

		uint32_t gridCol = gridColor;
		ar(cereal::make_nvp("gridColor", gridCol));
	}
};