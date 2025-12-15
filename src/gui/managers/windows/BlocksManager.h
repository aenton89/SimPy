//
// Created by tajbe on 10.11.2025.
//
#pragma once

#include <set>
#include "../../../core/structures/Blocks.h"
#include "../../../core/structures/Model.h"
#include "managers/BasicManager.h"
// #include <imgui.h>
// #include <cereal/types/set.hpp>
// #include <cereal/archives/xml.hpp>



/*
 * manages blocks operations - moving, selection and overall existence
 */
class BlocksManager : public BasicManager {
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

	// drawing blocks
	void drawBlock(const std::shared_ptr<Block> &box);
	// moving, selection
	void clearSelectedBlocks(const ImGuiIO& io);
	void duplicateSelectedBlocks(const ImGuiIO& io);
	void deleteSelectedBlocks(const ImGuiIO& io);
	void selectAllBlocks(const ImGuiIO& io);

	template<class Archive>
	void serialize(Archive& ar) {
		ar(cereal::base_class<BasicManager>(this));

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