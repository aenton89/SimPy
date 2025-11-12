//
// Created by tajbe on 10.11.2025.
//
#pragma once

#include <imgui.h>
#include <set>
#include <cereal/archives/xml.hpp>
#include "../../core/structures/Blocks.h"
#include "../../core/structures/Model.h"

class GUICore;



/*
 * manages blocks operations - moving, selection and overall existence
 */
class BlocksManager {
private:
	class GUICore* guiCore = nullptr;

public:
	// dla zaznaczania wielu box'ów
	std::set<int> selectedBlocks;
	bool isMultiSelectMode = false;
	// grupowe przeciąganie zaznaczonych boxów
	bool isGroupDragging = false;
	ImVec2 groupDragStartMousePos;
	std::unordered_map<int, ImVec2> groupInitialPositions;
	// dragging vs zooming -> problem który fix'ujemy
	bool isDraggingWindow = false;
	int draggedWindowId = -1;

	void setGUICore(GUICore* gui);

	// drawing blocks
	void drawBlock(Block& box);
	// moving, selection
	void clearSelectedBlocks(const ImGuiIO& io);
	void duplicateSelectedBlocks(const ImGuiIO& io);
	void deleteSelectedBlocks(const ImGuiIO& io);
	void selectAllBlocks(const ImGuiIO& io);

	template<class Archive>
	void serialize(Archive& ar) {
		ar(CEREAL_NVP(selectedBlocks),
		   CEREAL_NVP(isMultiSelectMode));

		// reset runtime state
		if constexpr (Archive::is_loading::value) {
			isDraggingWindow = false;
			draggedWindowId = -1;
			isGroupDragging = false;
			groupInitialPositions.clear();
		}
	}
};