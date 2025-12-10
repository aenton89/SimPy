//
// Created by tajbe on 07.11.2025.
//
#include "FileManager.h"
#include <GLFW/glfw3.h>
#include "../../GUICore.h"



bool FileManager::saveToXML(const std::string &filename, GUICore& gui) {
	std::string tempFilename = filename + ".tmp";
	std::string backupFilename = filename + ".bak";

	try {
		// scope jest potrzebny, bo dzięki temu destruktory zamkną i zapiszą plik przed rename
		{
			// zapisz do pliku tymczasowego
			std::ofstream file(tempFilename, std::ios::binary);
			if (!file.is_open()) {
				std::cerr << "ERR: can't open temp file " << tempFilename << "\n";
				return false;
			}

			cereal::XMLOutputArchive archive(file);
			archive(cereal::make_nvp("GUICore", gui));
		}

		// jeśli istnieje stary plik, zrób backup
		if (std::filesystem::exists(filename)) {
			// usuń stary backup jeśli istnieje
			if (std::filesystem::exists(backupFilename))
				std::filesystem::remove(backupFilename);

			// przenieś aktualny plik do backupu
			std::filesystem::rename(filename, backupFilename);
		}

		// zamień temp na główny plik
		std::filesystem::rename(tempFilename, filename);

		std::cout << "Save successful to " << filename << "\n";
		return true;

	} catch (const std::exception& e) {
		// spróbuj posprzątać
		if (std::filesystem::exists(tempFilename)) {
			try {
				std::filesystem::remove(tempFilename);
			} catch (...) {
				// ignoruj błędy przy czyszczeniu; pozdro z fartem
			}
		}

		std::cerr << "ERR: save error - " << e.what() << "\n";
		return false;
	}
}

bool FileManager::loadFromXML(const std::string &filename, GUICore& gui) {
	std::string backupFilename = filename + ".bak";

	auto tryLoad = [this, &gui](const std::string& fname) -> bool {
		try {
			std::ifstream file(fname, std::ios::binary);
			if (!file.is_open())
				return false;

			cereal::XMLInputArchive archive(file);
			archive(cereal::make_nvp("GUICore", gui));

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
	// tytuł okna; domyślna ścieżka; filtr; opcje (none = pojedynczy plik, multiselect = wiele plików)
	auto selection = pfd::open_file(
		"Select a file",
		".",
		{"XML Files", "*.xml", "All Files", "*"},
		pfd::opt::none
	).result();

	if (!selection.empty()) {
		const std::string& filepath = selection[0];
		std::cout << "Selected file: " << filepath << "\n";

		if (loadFromXML(filepath, *guiCore)) {
			std::cout << "File loaded successfully!\n";
			currentFilePath = filepath;
			hasUnsavedChanges = false;
		} else {
			std::cerr << "ERR: failed to load file!\n";
			// komunikat błędu
			pfd::message("Error", "Failed to load file: " + filepath, pfd::choice::ok, pfd::icon::error);
		}
	} else {
		std::cout << "No file selected\n";
	}
}

void FileManager::saveFileDialog() {
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
		if (filepath.find(".xml") == std::string::npos) {
			filepath += ".xml";
		}

		std::cout << "Save to: " << filepath << "\n";

		if (saveToXML(filepath, *guiCore)) {
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

void FileManager::saveFile() {
	// jeśli nie ma aktualnej ścieżki, otwórz dialog "Save As"
	if (currentFilePath.empty()) {
		saveFileDialog();
		return;
	}

	// zapisz do bieżącego pliku
	std::cout << "Saving to: " << currentFilePath << "\n";

	if (saveToXML(currentFilePath, *guiCore)) {
		std::cout << "File saved successfully!\n";
		hasUnsavedChanges = false;
	} else {
		std::cerr << "ERR: failed to save file!\n";
		pfd::message("Error", "Failed to save file: " + currentFilePath, pfd::choice::ok, pfd::icon::error);
	}
}

void FileManager::exitFile() {
	if (hasUnsavedChanges) {
		auto result = pfd::message(
			"Unsaved changes",
			"Do you want to save before exiting?",
			pfd::choice::yes_no_cancel,
			pfd::icon::question
		).result();

		if (result == pfd::button::yes) {
			saveFile();
			glfwSetWindowShouldClose(guiCore->window, GLFW_TRUE);
		} else if (result == pfd::button::no) {
			glfwSetWindowShouldClose(guiCore->window, GLFW_TRUE);
		}
		// cancel - nie rób nic
	} else {
		glfwSetWindowShouldClose(guiCore->window, GLFW_TRUE);
	}
}

void FileManager::newFile() {
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
			saveFile();
		else if (result == pfd::button::cancel) {
			ImGui::EndMenu();
			// anuluj tworzenie nowego
			return;
		}
	}

	// i na koniec: wyczyść dane
	guiCore->model = Model();
	currentFilePath.clear();
	hasUnsavedChanges = false;
}

// zapis pod CTRL+S, zapis jako pod CTRL+SHIFT+S
void FileManager::saveStateShortcut(const ImGuiIO &io) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)){
		if (io.KeyShift)
			saveFileDialog();
		saveFile();
	}
}

// wczytywanie pod CTRL+O
void FileManager::loadStateShortcut(const ImGuiIO &io) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
		openFileDialog();
}

// wyjście pod CTRL+W
void FileManager::exitFileShortcut(const ImGuiIO &io) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W, false))
		exitFile();
}

// nowy plik pod CTRL+N
void FileManager::newFileShortcut(const ImGuiIO &io) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false))
		newFile();
}