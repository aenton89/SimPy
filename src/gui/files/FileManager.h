//
// Created by tajbe on 07.11.2025.
//
#pragma once

#include <imgui.h>
#include <cereal/archives/xml.hpp>
#include <GLFW/glfw3.h>

class GUICore;



/*
 * manages file operations: new, open, save, save as
 */
class FileManager {
private:
	class GUICore* guiCore = nullptr;

public:
	std::string currentFilePath;
	bool hasUnsavedChanges;

	void setGUICore(GUICore* gui);

	// zapis/odczyt stanu do/z pliku
	bool saveToXML(const std::string& filename);
	bool loadFromXML(const std::string& filename);
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
		// reset runtime state
		if constexpr (Archive::is_loading::value) {
			hasUnsavedChanges = false;
		}
	}
};