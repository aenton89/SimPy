//
// Created by tajbe on 10.11.2025.
//
#pragma once

#include <imgui.h>
#include <cereal/archives/xml.hpp>



struct UIPreferences {
	bool lightMode = false;

	bool gridEnabled = true;
	float gridSpacing = 50.0f;
	float gridThickness = 1.0f;
	ImU32 gridColor = IM_COL32(0, 0, 0, 100);

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