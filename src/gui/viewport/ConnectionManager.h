//
// Created by tajbe on 12.11.2025.
//
#pragma once

#include <imgui.h>
#include <optional>
#include <cereal/archives/xml.hpp>
#include "../core/structures/BasicBlock.h"
#include "../core/structures/Connection.h"

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
 * pomocnicza struktura do przechowywania informacji o węźle w krzywej
 */
struct DraggingNode {
	// wskaźnik do Connection w Model
	Connection* connection;
	// indeks węzła w controlNodes
	int nodeIndex;
};



/*
 * manages drawing connection between blocks
 */
class ConnectionManager {
private:
	GUICore* guiCore = nullptr;

	// jakieś helpery
	ImVec2 worldToScreen(const ImVec2& worldPos) const;
	ImVec2 screenToWorld(const ImVec2& screenPos) const;
	void drawSingleConnection(Connection& conn, const ImVec2& mousePos, const ImVec2& mousePosWorld, ImDrawList* drawList);
	void handleNodeDragging(const ImVec2& mousePosWorld);
	void handleConnectionCreation(const ImVec2& mousePos, ImDrawList* drawList);

public:
	std::optional<int> draggingFrom;
	std::optional<DraggingNode> draggingNode;

	void setGUICore(GUICore* gui);

	void drawConnections();

	template<class Archive>
	void serialize(Archive& ar) {
		if constexpr (Archive::is_loading::value) {
			// reset runtime state
			draggingFrom.reset();
			draggingNode.reset();
		}
	}
};