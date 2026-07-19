//
// Created by tajbe on 07.11.2025.
//
#include "FileManager.h"
#include <GLFW/glfw3.h>
#include "BluePrintTab.h"
#include "../ide/NotebookTab.h"
#include "../blueprints/gui/BluePrintTab.h"
#include "../../tabs/TabManager.h"
#include <nlohmann/json.hpp>



bool FileManager::saveToXML(const std::string &filename, BluePrintTab& gui) {
	std::string tempFilename = filename + ".tmp";
	std::string backupFilename = filename + ".bak";

	try {
		{
			std::ofstream file(tempFilename, std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "ERR: can't open temp file " << tempFilename << "\n";
				return false;
			}

			cereal::XMLOutputArchive archive(file);
			archive(cereal::make_nvp("blueprintTab", gui));
		}

		if (std::filesystem::exists(filename)) {
			if (std::filesystem::exists(backupFilename))
				std::filesystem::remove(backupFilename);

			std::filesystem::rename(filename, backupFilename);
		}

		// zamień temp na główny plik
		std::filesystem::rename(tempFilename, filename);

		std::cout << "Save successful to " << filename << "\n";
		return true;

	} catch (const std::exception& e) {
		if (std::filesystem::exists(tempFilename)) {
			try {
				std::filesystem::remove(tempFilename);
			} catch (...) {

			}
		}

		std::cerr << "ERR: save error - " << e.what() << "\n";
		return false;
	}
}

bool FileManager::loadFromXML(const std::string &filename, BluePrintTab& gui) {
	std::string backupFilename = filename + ".bak";

	auto tryLoad = [&gui](const std::string& fname) -> bool {
		try {
			std::ifstream file(fname, std::ios::binary);
			if (!file.is_open())
				return false;

			cereal::XMLInputArchive archive(file);
			archive(cereal::make_nvp("blueprintTab", gui));

			std::cout << "Loaded successfully from " << fname << "\n";
			return true;

		} catch (const std::exception& e) {
			std::cerr << "ERR: failed to load from: " << fname << " - " << e.what() << "\n";
			return false;
		}
	};

	// najpierw spróbuj głównego pliku
	if (tryLoad(filename))
		return true;

	// jeśli się nie udało, spróbuj backup
	std::cerr << "Trying backup file...\n";
	if (std::filesystem::exists(backupFilename))
		return tryLoad(backupFilename);

	std::cerr << "ERR: no valid save file found!\n";
	return false;
}

void FileManager::openFileDialog() {
	auto selection = pfd::open_file("Open File", "",
	   {"Project Files", "*.xml *.py *.txt *.ipynb", "All Files", "*"}
	).result();

	if (selection.empty()) return;

	std::string filePath = selection[0];
	std::filesystem::path p(filePath);
	std::string extension = p.extension().string();

	if (extension == ".xml") {
		if (tabManager) {
			// Open the new tab first
			tabManager->openTab<BluePrintTab>();

			std::cout << "Selected file: " << filePath << "\n";

			auto* newTab = dynamic_cast<BluePrintTab*>(tabManager->getTabs().back().get());

			if (newTab) {
				if (loadFromXML(filePath, *newTab)) { // Passing by reference to loadFromXML
					std::cout << "File loaded successfully!\n";
					currentFilePath = filePath;
					hasUnsavedChanges = false;
				} else {
					std::cerr << "ERR: failed to load file!\n";
					pfd::message("Error", "Failed to load file: " + filePath, pfd::choice::ok, pfd::icon::error);
				}
			} else {
				std::cerr << "ERR: The newly created tab is not a BluePrintTab!\n";
			}
		}
	}
	else if (extension == ".ipynb") {
		if (tabManager) {
			tabManager->openTab<NotebookTab>();

		}
	}
}

template<>
void FileManager::saveFileDialog<BluePrintTab>(BluePrintTab& blueprintTab) {
	// domyślna ścieżka - jeśli nie istnieje to "untitled.xml"
	std::string defaultPath = currentFilePath.empty() ? "untitled.xml" : currentFilePath;
	// tytuł okna; domyślna ścieżka; filtr; pytaj o nadpisanie
	auto destination = pfd::save_file(
		"Save file",
		defaultPath,
		{"XML Files", "*.xml", "All Files", "*"},
		pfd::opt::force_overwrite
	).result();

	if (!destination.empty()) {
		std::string filepath = destination;

		// dodaj rozszerzenie .xml jeśli nie ma
		if (filepath.find(".xml") == std::string::npos)
			filepath += ".xml";

		std::cout << "Save to: " << filepath << "\n";

		if (saveToXML(filepath, blueprintTab)) { // tu byl ptr
			std::cout << "File saved successfully!\n";
			currentFilePath = filepath;
			hasUnsavedChanges = false;
			// pokaż komunikat sukcesu
			pfd::message("Success", "File saved successfully!", pfd::choice::ok, pfd::icon::info);
		} else {
			std::cerr << "Failed to save file!\n";
			pfd::message("Error", "Failed to save file: " + filepath, pfd::choice::ok, pfd::icon::error);
		}
	} else {
		std::cout << "Save cancelled\n";
	}
}

// metoda dla blueprinta
template<>
void FileManager::saveFile<BluePrintTab>(BluePrintTab& blueprintTab) {
	// jeśli nie ma aktualnej ścieżki, otwórz dialog "Save As"
	if (currentFilePath.empty()) {
		saveFileDialog(blueprintTab);
		return;
	}

	// zapisz do bieżącego pliku
	std::cout << "Saving to: " << currentFilePath << "\n";

	if (saveToXML(currentFilePath, blueprintTab)) { // tu byl ptr
		std::cout << "File saved successfully!\n";
		hasUnsavedChanges = false;
	} else {
		std::cerr << "ERR: failed to save file!\n";
		pfd::message("Error", "Failed to save file: " + currentFilePath, pfd::choice::ok, pfd::icon::error);
	}
}

// metoda dla notebooka
template<>
void FileManager::saveFile<NotebookTab>(NotebookTab& notebookTab) {
	// tworzenie mapy cell
	const std::vector<std::unique_ptr<BaseCell>>& cells = notebookTab.getNotebookTab()->getCells();

	std::map<int, CellData> cellMap;
	int i = 0;

	for (auto& cell : cells) {
		CellData cellData;

		if (cell->getType() == CellType::CodeCell) {
			const auto& codeCell = dynamic_cast<const CodeCell&>(*cell);
			cellData.cell_type = "code";
			cellData.source = codeCell.getInputText();
			cellData.outputs = codeCell.getOutputText();
		} else {
			const auto& baseCell = dynamic_cast<const BaseCell&>(*cell);
			cellData.cell_type = "markdown";
			cellData.source = baseCell.getInputText();
		}
		cellMap[i++] = cellData;
	}

	// zapis do json-a

	nlohmann::json j;
	j['cells'] = nlohmann::json::array();

	for (const auto& [index, cell] : cellMap) {
		nlohmann::json cellJson;
		cellJson["cell_type"] = cell.cell_type;
		cellJson["metadata"] = nlohmann::json::object();
		cellJson["source"] = nlohmann::json::array();
		cellJson["source"].push_back(cell.source);

		if (cell.cell_type == "code") {
			cellJson["outputs"] = nlohmann::json::array();
			for (const auto& out : cell.outputs) {
				nlohmann::json outputJson;
				outputJson["output_type"] = "stream";
				outputJson["name"] = "stdout";
				outputJson["text"] = out;
				cellJson["outputs"].push_back(outputJson);
			}
			cellJson["execution_count"] = nullptr;
		}

		j["cells"].push_back(cellJson);

		// TODO te metadane trzeba poprawic
		j["metadata"] = {
			{"kernelspec", {
	                {"display_name", "Python 3"},
					{"language", "python"},
					{"name", "python3"}
			}},
			{"language_info", {
	                {"name", "python"},
					{"version", "3.10"}
			}}
		};
		j["nbformat"] = 4;
		j["nbformat_minor"] = 5;

		std::ofstream file(currentFilePath);
		file << j.dump(4);
	}
}


template<>
void FileManager::exitFile<BluePrintTab>(BluePrintTab& blueprintTab) {
	if (hasUnsavedChanges) {
		auto result = pfd::message(
			"Unsaved changes",
			"Do you want to save before exiting?",
			pfd::choice::yes_no_cancel,
			pfd::icon::question
		).result();

		if (result == pfd::button::yes) {
			saveFile(blueprintTab);
			glfwSetWindowShouldClose(blueprintTab.tabManager->window, GLFW_TRUE);
		} else if (result == pfd::button::no) {
			glfwSetWindowShouldClose(blueprintTab.tabManager->window, GLFW_TRUE);
		}
		// cancel - nie rób nic
	} else {
		glfwSetWindowShouldClose(blueprintTab.tabManager->window, GLFW_TRUE);
	}
}

template<>
void FileManager::newFile<BluePrintTab> (BluePrintTab& blueprintTab) {
	// wyczyść projekt
	if (hasUnsavedChanges) {
		// zapytaj czy zapisać przed utworzeniem nowego
		auto result = pfd::message(
			"Unsaved changes",
			"Do you want to save before creating new file?",
			pfd::choice::yes_no_cancel,
			pfd::icon::question
		).result();

		if (result == pfd::button::yes)
			saveFile(blueprintTab);
		else if (result == pfd::button::cancel) {
			ImGui::EndMenu();
			// anuluj tworzenie nowego
			return;
		}
	}

	// i na koniec: wyczyść dane
	blueprintTab.model = Model();
	currentFilePath.clear();
	hasUnsavedChanges = false;
}

// zapis pod CTRL+S, zapis jako pod CTRL+SHIFT+S
template<>
void FileManager::saveStateShortcut<BluePrintTab> (const ImGuiIO &io, BluePrintTab& blueprintTab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)){
		if (io.KeyShift)
			saveFileDialog(blueprintTab);
		saveFile(blueprintTab);
	}
}

// wczytywanie pod CTRL+O
template<>
void FileManager::loadStateShortcut<BluePrintTab>(const ImGuiIO &io, BluePrintTab& blueprintTab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
		openFileDialog();
}

// wyjście pod CTRL+W
template<>
void FileManager::exitFileShortcut<BluePrintTab>(const ImGuiIO &io, BluePrintTab& blueprintTab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W, false))
		exitFile(blueprintTab);
}

// nowy plik pod CTRL+N
template<>
void FileManager::newFileShortcut<BluePrintTab>(const ImGuiIO &io, BluePrintTab& blueprintTab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false))
		newFile(blueprintTab);
}

void FileManager::setTabManager(TabManager* tabManager) {
	this->tabManager = tabManager;
}
////////////////////////////////////////////////////////// Metody dla Notebooka /////////////////////////////////////////////////////


