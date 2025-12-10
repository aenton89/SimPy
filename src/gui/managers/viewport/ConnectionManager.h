//
// Created by tajbe on 12.11.2025.
//
#pragma once

#include <optional>
#include "../BasicManager.h"
#include "../../../core/structures/Connection.h"
// #include <imgui.h>



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

// TODO: zmiany - jak coś to jest struktura do przechowywania informacji o obecnie tworzonym node'zie
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
 * TODO: dodać usuwanie całych połączeń, a nie, że pierwsze każdy node z osobna
 * TODO: po run pozycje node'ów się resetują - trzeba to naprawić
 */
class ConnectionManager : public BasicManager {
private:
	// jakieś helpery
	// TODO: zmiany - czy te dwa są potrzebne? - jeśli tak to przenieść poza klase
	[[nodiscard]]
	ImVec2 worldToScreen(const ImVec2& worldPos) const;
	[[nodiscard]]
	ImVec2 screenToWorld(const ImVec2& screenPos) const;
	// TODO: zmiany - rysowanie pojedyńczego połączenia - iterując po wektor_punktow_kontrolnych
	void drawSingleConnection(Connection& conn, const ImVec2& mousePos, const ImVec2& mousePosWorld, ImDrawList* drawList);
	// TODO: zmiany
	void handleNodeDragging(const ImVec2& mousePosWorld);
	void handleConnectionCreation(const ImVec2& mousePos, ImDrawList* drawList);

public:
	// TODO: zmiany - dodać strukture do przechowywania punktów kontrolnych? - std::map<std::shared_ptr<Connection> wskaznik_polaczenia, std::vector<std::ImVec2> wektor_punktow_kontrolnych>
	std::optional<int> draggingFrom;
	std::optional<DraggingNode> draggingNode;

	void drawConnections();

	template<class Archive>
	void serialize(Archive& ar) {
		ar(cereal::base_class<BasicManager>(this));

		if constexpr (Archive::is_loading::value) {
			// reset runtime state
			draggingFrom.reset();
			draggingNode.reset();
		}
	}
};