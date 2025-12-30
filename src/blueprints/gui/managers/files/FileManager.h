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
class FileManager : public BasicManager {
public:
	std::string currentFilePath;
	bool hasUnsavedChanges;

	// zapis/odczyt stanu do/z pliku
	bool saveToXML(const std::string& filename, GUICore& gui);
	bool loadFromXML(const std::string& filename, GUICore& gui);
	// do wyboru i odczytu/zapisu plików (otwieraja też okienka dialogowe)
	void openFileDialog();
	void saveFileDialog();
	void saveFile();
	// wyjście z pliku i zrobienie nowego
	void exitFile();
	void newFile();
	// skróty klawiszowe
	void saveStateShortcut(const ImGuiIO &io);
	void loadStateShortcut(const ImGuiIO &io);
	void exitFileShortcut(const ImGuiIO &io);
	void newFileShortcut(const ImGuiIO &io);

	template<class Archive>
	void serialize(Archive& ar) {
		ar(cereal::base_class<BasicManager>(this));

		// reset runtime state
		if constexpr (Archive::is_loading::value) {
			hasUnsavedChanges = false;
		}
	}
};