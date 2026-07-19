//
// Created by tajbe on 07.11.2025.
//
#pragma once

// #include "../tabs/TabManager.h"
#include "managers/BasicManager.h"
// #include <imgui.h>
// #include <cereal/archives/xml.hpp>
#include <nlohmann/json.hpp>
#include <map>
#include "../ide/gui/code_editor/cells/baseCell.h"
// #include "../ide/NotebookTab.h"
#include "../ide/gui/code_editor/notebookTile.h"
#include "../ide/gui/code_editor/cells/codeCell.h"
#include "../ide/gui/code_editor/cells/mardownCell.h"

class TabManager;

class NotebookTab;


// do przechowywania danych celli dla jupitera
struct CellData
{
	std::string cell_type;
	std::string source;
	std::string outputs;
};

/*
 * manages file operations: new, open, save, save as
 */
class FileManager {
private:

	// Te metody wewnętrzne zostają (będą wywoływane wewnątrz specjalizacji dla BluePrintTab)
	bool saveToXML(const std::string& filename, BluePrintTab& gui);
	bool loadFromXML(const std::string& filename, BluePrintTab& gui);

	// Metody do otierania notebooka
	bool loadFromIpynb(const std::string& filename, NotebookTab& gui);
	bool saveToIpynb(const std::string& filename, NotebookTab& gui);

	// Zeby file dialog mogl otierac dowalne pliki
	TabManager* tabManager;



public:
	std::string currentFilePath;
	bool hasUnsavedChanges;

	void openFileDialog();

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

	void setTabManager(TabManager* tabManager);
};