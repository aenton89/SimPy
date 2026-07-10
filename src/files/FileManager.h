//
// Created by tajbe on 07.11.2025.
//
#pragma once

#include "managers/BasicManager.h"
// #include <imgui.h>
// #include <cereal/archives/xml.hpp>



/*
 * manages file operations: new, open, save, save as
 */
class FileManager {
protected:
	// Te metody wewnętrzne zostają (będą wywoływane wewnątrz specjalizacji dla BluePrintTab)
	bool saveToXML(const std::string& filename, BluePrintTab& gui);
	bool loadFromXML(const std::string& filename, BluePrintTab& gui);

public:
	std::string currentFilePath;
	bool hasUnsavedChanges;

	template<typename T>
	void openFileDialog(T& activeTab);

	template<typename T>
	void saveFileDialog(T& activeTab);

	template<typename T>
	void saveFile(T& activeTab);

	template<typename T>
	void exitFile(T& activeTab);

	template<typename T>
	void newFile(T& activeTab);

	// Skróty klawiszowe
	template<typename T>
	void saveStateShortcut(const ImGuiIO &io, T& activeTab);

	template<typename T>
	void loadStateShortcut(const ImGuiIO &io, T& activeTab);

	template<typename T>
	void exitFileShortcut(const ImGuiIO &io, T& activeTab);

	template<typename T>
	void newFileShortcut(const ImGuiIO &io, T& activeTab);

	template<class Archive>
	void serialize(Archive& ar) {
		ar(cereal::base_class<BasicManager>(this));

		if constexpr (Archive::is_loading::value) {
			hasUnsavedChanges = false;
		}
	}
};