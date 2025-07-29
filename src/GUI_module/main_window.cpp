//
// Created by patryk on 25.07.25.
//

#include "main_window.h"
#include "imgui.h"
#include "PythonKernel.h"
#include "CodeCell.h"
#include "MarkDownCell.h"
#include <imfilebrowser.h>
#include <map>
#include <nlohmann/json.hpp>
#include "file_menager.h"
#include <filesystem>

using json = nlohmann::json;

namespace fs = std::filesystem;

///////////////////////////////////////////////////////////////// Custom open FileBrowser /////////////////////////////////////////////////////////////
OpenFileBrowser::OpenFileBrowser()
    :  fileDialog(ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_SelectDirectory)
{
    fileDialog.SetTitle("Select File");
}
void OpenFileBrowser::Open()
{
    fileDialog.Open();
}
void OpenFileBrowser::Render()
{
    fileDialog.Display();
    if (fileDialog.HasSelected())
    {
        *this->path = fs::path(fileDialog.GetSelected());
        fileDialog.ClearSelected();
    }
}

void OpenFileBrowser::set_selectPath(fs::path& path)
{
    this->path = &path;
}


static OpenFileBrowser browser;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MainWindow::MainWindow()
{
    this->cells.clear();
    initKernel("/home/patryk/Desktop/venv/ML_venv/bin/python3");
}

MainWindow::~MainWindow()
{
    delete this->kernel;
}

/*
 * Fun is use to resize od main window if user change a screen
 *@param
 *  ImVec2 (wigth of screen, height of screen
 */

void MainWindow::refresDisp(ImVec2 dispSize)
{
    this->width = dispSize.x;
    this->height = dispSize.y;
}

/*
 * Fun convert Python dict to cpp map.
 * @param
 *  std::string dict - this is a representation of pytho dict as string
 * @return
 *  std::map<std::string, std::string> - convetd dict to map
 */
std::map<std::string, std::string> MainWindow::dict2map(const std::string dict)
{
    std::map<std::string, std::string> map;

    if (dict.empty()) return map;

    // Kopia wejściowego stringa
    std::string modified_dict = dict;

    // Usuń wszystko po [[OK]]
    size_t ok_pos = modified_dict.find("[[OK]]");
    if (ok_pos != std::string::npos) {
        modified_dict = modified_dict.substr(0, ok_pos);
    }

    // Usuń białe znaki z początku i końca
    modified_dict.erase(0, modified_dict.find_first_not_of(" \t\n\r"));
    modified_dict.erase(modified_dict.find_last_not_of(" \t\n\r") + 1);

    // Usuń otaczające apostrofy
    if (!modified_dict.empty() &&
        modified_dict.front() == '\'' &&
        modified_dict.back() == '\'')
    {
        modified_dict = modified_dict.substr(1, modified_dict.size() - 2);
    }

    try {
        json parsed_json = json::parse(modified_dict);

        for (auto& [key, value] : parsed_json.items()) {
            if (value.is_string())
                map[key] = value.get<std::string>();
            else
                map[key] = value.dump();  // inne typy konwertujemy na string
        }
    } catch (const std::exception& e) {
        std::cerr << "Błąd podczas parsowania JSON-a: " << e.what() << std::endl;
    }

    return map;
}


ImVec4 MainWindow::mainWindow()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize   |
                             ImGuiWindowFlags_NoMove     |
                             ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("SimPy Code Editor", nullptr, flags);

    ImVec4 codeEditor_xywh = codeEditor();
    ImVec4 varTable_xywh = varTable();
    ImVec4 workSpace_xywh = workSpace();
    ImVec4 mainBar_xywh = menuBar();

    ImGui::End();

    ImVec2 WindowPos = ImGui::GetWindowPos();
    ImVec2 WindowSize = ImGui::GetWindowSize();

    return ImVec4(WindowPos.x, WindowPos.y, WindowSize.x, WindowSize.y);
}

/////////////////////////////////////////////// Kernel Setup///////////////////////////////////
void MainWindow::initKernel(std::string path) {
    delete this->kernel;
    this->kernel = new PythonKernel(path, "/home/patryk/CLionProjects/SimPy_gui_refactor/src/python_kernel/kernel.py");
    this->kernel->start();

    // podmieniamy kernel tylko tam, gdzie to ma sens (Cell)
    for (auto& cell : this->cells)
        cell->setKernel(this->kernel);

}

void MainWindow::stopKernel()
{
    if (this->kernel != nullptr)
        this->kernel->stop();
}
void MainWindow::resetKernel()
{
    if (this->kernel != nullptr)
        this->kernel->reset();
}
//////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////// Element of main window /////////////////////////////////////////////////
ImVec4 MainWindow::codeEditor()
{
    ImGui::SetCursorPos(ImVec2(this->width / 10, this->height / 10));

    ImVec2 childSize = ImVec2(this->width * 8 / 10, this->height - this->height / 10);

    ImGui::BeginChild("Code Editor", childSize, true,
        ImGuiChildFlags_ResizeY);

    ImGui::Text("Code Editor");

    if (ImGui::Button("Add Code Cell"))
        this->cells.push_back(std::make_unique<Cell>(*this->kernel));

    ImGui::SameLine();
    if (ImGui::Button("Add MarkDown Cell"))
        this->cells.push_back(std::make_unique<MarkDownCell>());


    for (int i = 0; i < (int)cells.size(); ++i) {
        ImGui::PushID(i);
        if (cells[i]->Draw(i)) {
            cells.erase(cells.begin() + i);
            --i;
        }
        ImGui::PopID();
    }

    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 size = ImGui::GetItemRectSize();

    ImGui::EndChild();

    return ImVec4(pos.x, pos.y, size.x, size.y);
}


ImVec4 MainWindow::menuBar()
{
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImVec2 childSize = ImVec2(this->width, this->height / 10);

    bool openOpenfile = false;

    if (ImGui::BeginChild("MenuBar", childSize, true, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            // --- MENU FILE ---
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::BeginMenu("New", "Ctrl+N"))
                {
                    // TODO: obsługa nowego pliku
                    if (ImGui::MenuItem("New porject"))
                    {

                    }

                    if (ImGui::MenuItem("Python file (.py)"))
                    {

                    }
                    if (ImGui::MenuItem("notebook file (.ipnb)"))
                    {

                    }
                    if (ImGui::MenuItem("SimPy (.spy)"))
                    {

                    }

                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                {
                    this->cells.clear();

                    openOpenfile = true;
                }

                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    // TODO: zapis pliku
                    std::cout << "[Menu] Save\n";
                    for (auto& pair: createIpynb_map(this->cells))
                    {
                        std::cout << pair.first << " -> " << pair.second.source << "\n";
                    }

                    writeIpynb("/home/patryk/Desktop/ipy_test.ipynb", createIpynb_map(this->cells));
                }

                if (ImGui::MenuItem("Save As..."))
                {
                    // TODO: obsługa zapisu jako
                    std::cout << "[Menu] Save As...\n";
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Exit", "Alt+F4"))
                {
                    // zamknięcie aplikacji
                    exit(0);
                }

                ImGui::EndMenu();
            }

            // --- MENU EDIT ---
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "Ctrl+Z"))
                {
                    std::cout << "[Menu] Undo\n";
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
                {
                    std::cout << "[Menu] Redo (disabled)\n";
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Cut", "Ctrl+X"))
                {
                    std::cout << "[Menu] Cut\n";
                }
                if (ImGui::MenuItem("Copy", "Ctrl+C"))
                {
                    std::cout << "[Menu] Copy\n";
                }
                if (ImGui::MenuItem("Paste", "Ctrl+V"))
                {
                    std::cout << "[Menu] Paste\n";
                }

                ImGui::EndMenu();
            }

            // --- MENU VIEW ---
            if (ImGui::BeginMenu("View"))
            {
                static bool showGrid = true;
                if (ImGui::MenuItem("Toggle Fullscreen"))
                {
                    std::cout << "[Menu] Toggle Fullscreen\n";
                }
                ImGui::MenuItem("Show Grid", nullptr, &showGrid);
                ImGui::EndMenu();
            }

            // --- MENU SETTINGS ---
            if (ImGui::BeginMenu("Settings"))
            {
                if (ImGui::MenuItem("Preferences"))
                {
                    std::cout << "[Menu] Preferences\n";
                }
                if (ImGui::MenuItem("Theme"))
                {
                    std::cout << "[Menu] Theme\n";
                }
                ImGui::EndMenu();
            }

            // --- MENU HELP ---
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("Documentation"))
                {
                    std::cout << "[Menu] Documentation\n";
                }
                if (ImGui::MenuItem("About"))
                {
                    std::cout << "[Menu] About\n";
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    static std::string pathFile;

    ///////////////////// Te gowna bedzie trzeba wyjebac na koniec
    browser.Render();


    /////////////////////// Otieranie pliku jupiter /////////////////////
    if (openOpenfile)
    {
        browser.Open();
        browser.set_selectPath(this->selected_path);
    }

    /*
    if (!this->selected_path.empty())
    {
        std::cout<< "PathFile" << this->selected_path << "\n";
        for (auto& pair : openIpynb(this->selected_path))
        {
            if (pair.second.cell_type == "code")
            {
                this->cells.push_back(std::make_unique<Cell>(*this->kernel));
                auto& cell = *this->cells.back();
                dynamic_cast<Cell&>(cell).setCodeInput(pair.second.source);
                dynamic_cast<Cell&>(cell).setCodeOutput(pair.second.outputs);
            }
            else
            {
                this->cells.push_back(std::make_unique<MarkDownCell>());
                auto& cell = *this->cells.back();
                dynamic_cast<MarkDownCell&>(cell).setInput(pair.second.source);
                dynamic_cast<MarkDownCell&>(cell).execMardown(); // poprawic
            }
        }
        pathFile.clear();
    }
    */
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    ImVec2 windowPos = ImGui::GetWindowPos();  // Pozycja okna
    ImVec2 windowSize = ImGui::GetWindowSize();  // Rozmiar okna

    ImVec2 comboPos = ImVec2(windowPos.x + windowSize.x * 0.7f, windowPos.y + windowSize.y * 0.5f);
    ImGui::SetCursorPos(comboPos);


    if (ImGui::Button("Reset kernel"))
    {
        resetKernel();
    }
    ImGui::SameLine();
    if (ImGui::Button("Run All"))
    {

    }

    ImGui::SameLine();

    std::vector<std::string> kernelsVec = parseKernelList(readJsonFile("/home/patryk/CLionProjects/SimPy_gui_refactor/src/json_config_module/kernel_config.json"));
    std::map<std::string, std::string> kernels;

    std::string kernel_name;
    std::vector<std::string> help_var;

    for (int i = 0; i < kernelsVec.size(); i++) {
        kernel_name = kernelsVec[i];
        help_var = split(kernel_name, '/');
        kernels[help_var[help_var.size() - 3]] = kernel_name;
    }

    kernels["Add Kernel"] = "Add Kernel";

    static size_t current_kernels = 0;
    bool openAddKernelPopup = false;

    ImGui::PushItemWidth(windowSize.x * 0.1f);

    static fs::path kernelPath;

    if (!kernels.empty()) {
        auto it = std::next(kernels.begin(), current_kernels);
        if (ImGui::BeginCombo("###Kernel Select", it->first.c_str())) {
            int index = 0;
            for (auto& kernel : kernels) {
                bool is_selected = (index == current_kernels);
                if (ImGui::Selectable(kernel.first.c_str(), is_selected)) {
                    current_kernels = index;
                    if (kernel.first == "Add Kernel") {
                        browser.Open();
                        browser.set_selectPath(kernelPath);
                    } else if (kernel.first != it->first.c_str()) {
                        initKernel(kernel.second);
                    }
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
                index++;
            }
            ImGui::EndCombo();
        }
    }

    if (!kernelPath.empty())
    {
        std::cout <<"kernel" << kernelPath << "\n";
        writeJsonFile(kernelPath, "/home/patryk/CLionProjects/SimPy_gui_refactor/src/json_config_module/kernel_config.json");
        kernelPath.clear();
    }

    ImGui::End();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImGui::EndChild();

    return ImVec4(pos.x, pos.y, size.x, size.y);
}

ImVec4 MainWindow::varTable()
{
    ImGui::SetCursorPos(ImVec2(this->width*9/10, this->height/10));
    ImVec2 childSize = ImVec2(this->width/10, this->height*9/10);

    ImGui::BeginChild("Var Table", childSize, true);

    ImGui::Text("Variable Table");

    static std::string selectedVar = "";
    static std::string selectedValue = "";

    std::map<std::string, std::string> varMap = dict2map(this->kernel->fetchNamespace());

    if (varMap.empty())
    {
        ImGui::TextDisabled("No variable table found");
    }
    else
    {
        if (ImGui::BeginTable("Var Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Var Name");
            ImGui::TableSetupColumn("Var Value");
            ImGui::TableHeadersRow();

            for (const auto &pair : varMap)
            {
                ImGui::TableNextRow();

                // kolumna 0 - nazwa zmiennej
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(pair.first.c_str(), false))
                {
                    selectedVar = pair.first;
                }

                // kolumna 1 - wartość zmiennej
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Selectable(pair.second.c_str(), false))
                {
                    selectedValue = pair.second;
                }
            }
            ImGui::EndTable();
        }
    }

    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec4 windowData(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

    ImGui::EndChild();
    return windowData;
}

/////////////////////////////////////////////////////////////////////////// Funkje zwiazne z workspace /////////////////////////////////////////////////////////////


void MainWindow::HandleDragDropTarget(const fs::path& target) {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
            const char* sourcePath = (const char*)payload->Data;
            fs::path source(sourcePath);
            fs::path destination = target / source.filename();

            try {
                if (!fs::exists(source)) {
                    std::cerr << "Źródło nie istnieje!\n";
                } else {
                    if (fs::exists(destination)) {
                        // unikamy kolizji nazw → dopisujemy "_copy"
                        destination = target / (source.filename().string() + "_copy");
                    }
                    fs::rename(source, destination); // realne przenoszenie
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Błąd przenoszenia: " << e.what() << "\n";
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void MainWindow::ShowDirectoryNode(const fs::directory_entry& entry, std::string& selected_path) {
    const std::string name = entry.path().filename().string();

    if (entry.is_directory()) {
        if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
            // Kliknięcia
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                selected_path = entry.path().string();
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                ImGui::OpenPopup("DirContextMenu");

            // Źródło przeciągania
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                std::string folderPath = entry.path().string();
                ImGui::SetDragDropPayload("FILE_PATH", folderPath.c_str(), folderPath.size() + 1);
                ImGui::Text("Przenoszę folder: %s", name.c_str());
                ImGui::EndDragDropSource();
            }

            // Obsługa drop target
            MainWindow::HandleDragDropTarget(entry.path());

            // Rekurencja
            for (const auto& child : fs::directory_iterator(entry.path()))
                ShowDirectoryNode(child, selected_path);

            ImGui::TreePop();
        }
    } else {
        // Plik jako element listy
        if (ImGui::Selectable(name.c_str()))
            selected_path = entry.path().string();

        // Drag source (plik)
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            std::string filePath = entry.path().string();
            ImGui::SetDragDropPayload("FILE_PATH", filePath.c_str(), filePath.size() + 1);
            ImGui::Text("Przenoszę plik: %s", name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            ImGui::OpenPopup("FileContextMenu");
    }
}

void MainWindow::ShowDirectoryTree(const fs::path& path, std::string& selected_path) {
    if (!fs::exists(path) || !fs::is_directory(path)) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Folder doesn't exist: %s", path.string().c_str());
        return;
    }

    for (const auto& entry : fs::directory_iterator(path)) {
        ShowDirectoryNode(entry, selected_path);
    }
}


ImVec4 MainWindow::workSpace()
{
    ImGui::SetCursorPos(ImVec2(0, this->height/10));
    ImVec2 childSize = ImVec2(this->width/10, this->height*9/10);

    static std::string selected_path;

    ImGui::BeginChild("WorkSpace", childSize, true);

    ImGui::Text("Work Space");

    if (ImGui::TreeNode(this->selected_path.filename().string().c_str())) {
        ShowDirectoryTree(this->selected_path, selected_path);

        if (!selected_path.empty()) {
            std::cout << selected_path << std::endl;
            selected_path.clear();
        }

        ImGui::TreePop();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec4 windowData(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

        ImGui::EndChild();

        return windowData;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


