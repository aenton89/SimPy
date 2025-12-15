//
// Created by tajbe on 25.10.2025.
//
#pragma once

#include <thread>
#include "managers/BasicManager.h"
// #include <imgui.h>
// #include <vector>
// #include <cereal/archives/xml.hpp>

class Model;



/*
 * helper structs, classes and constats required for docking
 */
constexpr ImVec2 DEFAULT_DOCKED_START_SIZE = ImVec2(195, 180);
constexpr ImVec2 DEFAULT_DOCKED_MENU_SIZE = ImVec2(245, 200);

enum class DockableWindowType {
	Menu,
	Start
};

enum class DockPosition {
	None,
	Left,
	Right,
	Top,
	Bottom
};

struct DockableWindow {
	DockPosition position = DockPosition::None;
	ImVec2 undockedPos = ImVec2(100, 100);
	ImVec2 dockedPos = ImVec2(100, 100);
	bool isDocked = false;

	template<class Archive>
	void serialize(Archive& ar) {
		int pos = static_cast<int>(position);
		ar(CEREAL_NVP(pos),
		   CEREAL_NVP(isDocked));

		// serializacja ImVec2 jako vector<float>
		std::vector<float> undocked = {undockedPos.x, undockedPos.y};
		std::vector<float> docked = {dockedPos.x, dockedPos.y};
		ar(cereal::make_nvp("undockedPos", undocked),
		   cereal::make_nvp("dockedPos", docked));

		// reset runtime state
		if constexpr (Archive::is_loading::value) {
			position = static_cast<DockPosition>(pos);
			undockedPos = ImVec2(undocked[0], undocked[1]);
			dockedPos = ImVec2(docked[0], docked[1]);
		}
	}
};



/*
 * manages windows which dock to screen edges with snap functionality
 */
class DockableWindowManager : public BasicManager {
private:
	DockableWindow menuWindow;
	DockableWindow startWindow;
	std::vector<DockableWindow> dockedWindows;
	// odległość od krawędzi do snap'owania
	float dockSnapDistance = 50.0f;
	// do śledzenia wyskości menu przy dockowaniu prawo/lewo
	float lastMenuHeight = 0.0f;
	bool menuHeightCalculated = false;

	// TODO: animacja dockowania
	bool menuAnimating = false;
	bool startAnimating = false;
	ImVec2 menuAnimationStart = ImVec2(0, 0);
	ImVec2 menuAnimationTarget = ImVec2(0, 0);
	ImVec2 startAnimationStart = ImVec2(0, 0);
	ImVec2 startAnimationTarget = ImVec2(0, 0);
	float menuAnimationProgress = 0.0f;
	float startAnimationProgress = 0.0f;
	float animationSpeed = 8.0f;

	[[nodiscard]]
	DockPosition checkDockPosition(ImVec2 windowPos, ImVec2 windowSize) const;
	[[nodiscard]]
	ImVec2 calculateDockedPosition(DockPosition position, DockableWindowType windowType) const;
	[[nodiscard]]
	ImVec2 calculateDockedSize(DockPosition position, DockableWindowType windowType) const;
	[[nodiscard]]
	static ImVec2 lerpVec2(const ImVec2& a, const ImVec2& b, float t);

public:
	void drawStartButton();
	void drawMenu();

	template<class Archive>
	void serialize(Archive& ar) {
		ar(cereal::base_class<BasicManager>(this));

		ar(CEREAL_NVP(menuWindow),
		   CEREAL_NVP(startWindow),
		   CEREAL_NVP(dockedWindows),
		   CEREAL_NVP(dockSnapDistance),
		   CEREAL_NVP(lastMenuHeight),
		   CEREAL_NVP(menuHeightCalculated));

		// reset animation state
		if constexpr (Archive::is_loading::value) {
			menuAnimating = false;
			startAnimating = false;
			menuAnimationProgress = 0.0f;
			startAnimationProgress = 0.0f;
		}
	}
};


