//
// Created by tajbe on 12.11.2025.
//
#pragma once

#include <imgui.h>
#include <optional>
#include <cereal/archives/xml.hpp>

class GUICore;



static float LengthSqr(const ImVec2& a, const ImVec2& b) {
	ImVec2 d = ImVec2(a.x - b.x, a.y - b.y);
	return d.x * d.x + d.y * d.y;
}

static ImVec2 BezierCubicCalc(const ImVec2& p1, const ImVec2& c1, const ImVec2& c2, const ImVec2& p2, float t) {
	float u = 1.0f - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	ImVec2 result = ImVec2(0, 0);
	result.x = uuu * p1.x;
	result.x += 3 * uu * t * c1.x;
	result.x += 3 * u * tt * c2.x;
	result.x += ttt * p2.x;

	result.y = uuu * p1.y;
	result.y += 3 * uu * t * c1.y;
	result.y += 3 * u * tt * c2.y;
	result.y += ttt * p2.y;

	return result;
}



/*
 * manages drawing connection between blocks
 */
class ConnectionManager {
private:
	class GUICore* guiCore = nullptr;

public:
	std::optional<int> dragging_from;

	void setGUICore(GUICore* gui);

	void drawConnections();

	template<class Archive>
	void serialize(Archive& ar) {
		if constexpr (Archive::is_loading::value) {
			// reset runtime state
			dragging_from.reset();
		}
	}
};