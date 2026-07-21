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


// save to .xml
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

// save do .ipynb
bool FileManager::saveToIpynb(const std::string& filename, NotebookTab& notebookTab) {
    const auto& cells = notebookTab.getNotebookTab()->getCells();

    nlohmann::json j;
    j["cells"] = nlohmann::json::array();

    for (const auto& cell : cells) {
        nlohmann::json cellJson;
        cellJson["metadata"] = nlohmann::json::object();

        // 1. Zapis kodu/tekstu komórki (dzielenie po liniach z \n dla zachowania formatu Jupyter)
        std::string inputText = cell->getInputText();
        nlohmann::json sourceArray = nlohmann::json::array();
        std::stringstream ss(inputText);
        std::string line;

        while (std::getline(ss, line)) {
            sourceArray.push_back(line + "\n");
        }
        cellJson["source"] = sourceArray.empty() ? nlohmann::json::array({""}) : sourceArray;

        // 2. Obsługa typu komórki oraz jej wyjść (outputs)
        if (cell->getType() == CellType::CodeCell) {
            cellJson["cell_type"] = "code";
            cellJson["execution_count"] = nullptr;
            cellJson["outputs"] = nlohmann::json::array();

            if (const auto* codeCell = dynamic_cast<const CodeCell*>(cell.get())) {

                // Zwykły tekst wyjściowy (stdout)
                std::string outputText = codeCell->getOutputText();
                if (!outputText.empty()) {
                    nlohmann::json outputJson;
                    outputJson["output_type"] = "stream";
                    outputJson["name"] = "stdout";

                    nlohmann::json textLines = nlohmann::json::array();
                    std::stringstream outSs(outputText);
                    std::string outLine;

                    while (std::getline(outSs, outLine)) {
                        textLines.push_back(outLine + "\n");
                    }
                    outputJson["text"] = textLines;

                    cellJson["outputs"].push_back(outputJson);
                }

                // Wyjście graficzne (Base64 obrazu png)
                std::string base64Img = codeCell->getBase64();
                if (!base64Img.empty()) {
                    nlohmann::json imgOutputJson;
                    imgOutputJson["output_type"] = "display_data";
                    imgOutputJson["metadata"] = nlohmann::json::object();
                    imgOutputJson["data"] = {
                        {"image/png", base64Img}
                    };

                    cellJson["outputs"].push_back(imgOutputJson);
                }
            }
        } else {
            cellJson["cell_type"] = "markdown";
        }

        j["cells"].push_back(cellJson);
    }

    // 3. Metadane pliku Notebook (.ipynb)
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

    // 4. Zapis gotowej struktury JSON do pliku
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    return true;
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

bool FileManager::loadFromIpynb(const std::string &filename, NotebookTab &gui) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << filename << std::endl;
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Błąd parsowania JSON: " << e.what() << std::endl;
        return false;
    }

    if (!j.contains("cells") || !j["cells"].is_array()) {
        return false;
    }

    std::vector<std::unique_ptr<BaseCell>> newCells;

    for (const auto& cellJson : j["cells"]) {
        std::string cellType = cellJson.value("cell_type", "");

        // 1. Odczyt tekstu wejściowego (source)
        std::string sourceText;
        if (cellJson.contains("source")) {
            if (cellJson["source"].is_array()) {
                for (const auto& line : cellJson["source"]) {
                    sourceText += line.get<std::string>();
                }
            } else if (cellJson["source"].is_string()) {
                sourceText = cellJson["source"].get<std::string>();
            }
        }

        if (cellType == "code") {
            auto codeCell = std::make_unique<CodeCell>(*gui.getNotebookTab()->getKernel());
            codeCell->setInputText(sourceText);

            std::string totalOutput;
            std::string base64Img;

            if (cellJson.contains("outputs") && cellJson["outputs"].is_array()) {
                for (const auto& output : cellJson["outputs"]) {
                    std::string outputType = output.value("output_type", "");

                    if (output.contains("text")) {
                        if (output["text"].is_array()) {
                            for (const auto& line : output["text"]) {
                                totalOutput += line.get<std::string>();
                            }
                        } else if (output["text"].is_string()) {
                            totalOutput += output["text"].get<std::string>();
                        }
                    }

                    if (output.contains("data") && output["data"].contains("image/png")) {
                        base64Img = output["data"]["image/png"].get<std::string>();
                    }
                }
            }

            if (!totalOutput.empty()) {
                codeCell->setOutputText(totalOutput);
            }

            if (!base64Img.empty()) {
                codeCell->processPlot(base64Img);
            }

            newCells.push_back(std::move(codeCell));

        } else if (cellType == "markdown") {
            auto mdCell = std::make_unique<MardownCell>();
            mdCell->setInputText(sourceText);
            newCells.push_back(std::move(mdCell));
        }
    }

    gui.getNotebookTab()->setCells(std::move(newCells));

    return true;
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

			std::cout << "Selected file: " << filePath << "\n";

			auto* newTab = dynamic_cast<NotebookTab*>(tabManager->getTabs().back().get());

			if (newTab) {
				if (loadFromIpynb(filePath, *newTab)) { // Passing by reference to loadFromXML
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
	} else {
		// tu bedzie reszta plikow kotre beda tratowane jako pliki tekstowe
	}
}


// okno dialogowe dla .xml
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

template<>
void FileManager::saveFileDialog<NotebookTab>(NotebookTab& notebooktab) {
	std::string defaultPath = currentFilePath.empty() ? "untitled.ipynb" : currentFilePath;

	auto destination = pfd::save_file(
		"Save file",
		defaultPath,
		{"XML Files", "*.ipynb", "All Files", "*"},
		pfd::opt::force_overwrite
		).result();

	if (!destination.empty()) {
		std::string filepath = destination;
		if (filepath.find(".ipynb") == std::string::npos)
			filepath += ".ipynb";

		std::cout << "Save to: " << filepath << "\n";

		if (FileManager::saveToIpynb(destination, notebooktab)) {
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
		saveFileDialog<BluePrintTab>(blueprintTab);
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
	if (currentFilePath.empty()) {
		saveFileDialog<NotebookTab>(notebookTab);
		return;
	}

	// zapisz do bieżącego pliku
	std::cout << "Saving to: " << currentFilePath << "\n";

	if (saveToIpynb(currentFilePath, notebookTab)) { // tu byl ptr
		std::cout << "File saved successfully!\n";
		hasUnsavedChanges = false;
	} else {
		std::cerr << "ERR: failed to save file!\n";
		pfd::message("Error", "Failed to save file: " + currentFilePath, pfd::choice::ok, pfd::icon::error);
	}
}

// metoda dla bluprinta
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

// metoda dla notebooka
template<>
void FileManager::exitFile<NotebookTab>(NotebookTab& notebooktab) {
	if (hasUnsavedChanges) {
		auto result = pfd::message(
			"Unsaved changes",
			"Do you want to save before exiting?",
			pfd::choice::yes_no_cancel,
			pfd::icon::question
		).result();

		if (result == pfd::button::yes) {
			saveFile(notebooktab);
			glfwSetWindowShouldClose(notebooktab.tabManager->window, GLFW_TRUE);
		} else if (result == pfd::button::no) {
			glfwSetWindowShouldClose(notebooktab.tabManager->window, GLFW_TRUE);
		}
		// cancel - nie rób nic
	} else {
		glfwSetWindowShouldClose(notebooktab.tabManager->window, GLFW_TRUE);
	}
}

// metoda dla blueprinta
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

// metoda dla notebooka
template<>
void FileManager::newFile<NotebookTab>(NotebookTab& notebooktab) {
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
			saveFile(notebooktab);
		else if (result == pfd::button::cancel) {
			ImGui::EndMenu();
			// anuluj tworzenie nowego
			return;
		}
	}

	currentFilePath.clear();
	hasUnsavedChanges = false;
}

// zapis pod CTRL+S, zapis jako pod CTRL+SHIFT+S
template<>
void FileManager::saveStateShortcut<BluePrintTab> (const ImGuiIO &io, BluePrintTab& blueprintTab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)){
		if (io.KeyShift)
			saveFileDialog<BluePrintTab>(blueprintTab);
		saveFile<BluePrintTab>(blueprintTab);
	}
}

template<>
void FileManager::saveStateShortcut<NotebookTab>(const ImGuiIO &io, NotebookTab& notebooktab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)){
		if (io.KeyShift)
			saveFileDialog<NotebookTab>(notebooktab);
		saveFile<NotebookTab>(notebooktab);
	}
}

// wczytywanie pod CTRL+O
template<>
void FileManager::loadStateShortcut<BluePrintTab>(const ImGuiIO &io, BluePrintTab& blueprintTab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
		openFileDialog();
}

template<>
void FileManager::loadStateShortcut<NotebookTab>(const ImGuiIO &io, NotebookTab& notebooktab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
		openFileDialog();
}

// wyjście pod CTRL+W
template<>
void FileManager::exitFileShortcut<BluePrintTab>(const ImGuiIO &io, BluePrintTab& blueprintTab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W, false))
		exitFile<BluePrintTab>(blueprintTab);
}

template<>
void FileManager::exitFileShortcut<NotebookTab>(const ImGuiIO &io, NotebookTab& notebooktab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W, false))
		exitFile<NotebookTab>(notebooktab);
}

// nowy plik pod CTRL+N
template<>
void FileManager::newFileShortcut<BluePrintTab>(const ImGuiIO &io, BluePrintTab& blueprintTab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false))
		newFile<BluePrintTab>(blueprintTab);
}

template<>
void FileManager::newFileShortcut<NotebookTab>(const ImGuiIO &io, NotebookTab& notebooktab) {
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false))
		newFile<NotebookTab>(notebooktab);
}

void FileManager::setTabManager(TabManager* tabManager) {
	this->tabManager = tabManager;
}

// uniwersalne metody dla
void FileManager::openFolderDialog() {
	auto destination = pfd::select_folder("Select Folder", "C:\\").result();

	if (destination.empty()) return;

	this->folderPath = destination;
	std::cout << this->folderPath << std::endl;
}


