//
// Created by tajbe on 25.10.2025.
//
#pragma once

#include <imgui.h>
#include <vector>
#include <cereal/archives/xml.hpp>
#include <thread>

class GUICore;
class Model;



/*
 * helper structs, classes and constats required for docking
 */
const ImVec2 DEFAULT_DOCKED_START_SIZE = ImVec2(195, 180);
const ImVec2 DEFAULT_DOCKED_MENU_SIZE = ImVec2(245, 200);

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
class DockableWindowManager {
private:
	// forward declaration + wskaźnik do rodzica
	class GUICore* guiCore = nullptr;

	DockableWindow menuWindow;
	DockableWindow startWindow;
	std::vector<DockableWindow> dockedWindows;
	// odległość od krawędzi do snap'owania
	float dockSnapDistance = 50.0f;

	ImVec2 calculateDockedPosition(DockPosition position, DockableWindowType windowType);
	ImVec2 calculateDockedSize(DockPosition position, DockableWindowType windowType);
	DockPosition checkDockPosition(ImVec2 windowPos, ImVec2 windowSize);

public:
	// i funkcja która go ustawi
	void setGUICore(GUICore* gui);

	void drawStartButton();
	void drawMenu();

	template<class Archive>
	void serialize(Archive& ar) {
		ar(CEREAL_NVP(menuWindow),
		   CEREAL_NVP(startWindow),
		   CEREAL_NVP(dockedWindows),
		   CEREAL_NVP(dockSnapDistance));
	}
};


