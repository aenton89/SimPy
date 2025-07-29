//
// Created by patryk on 25.07.25.
//

#include "imgui.h"
#include <string>
#include "PythonKernel.h"
#include <vector>
#include <memory>
#include "BaseCell.h"
#include "file_menager.h"
#include <map>
#include "openFile.h"
#include <imfilebrowser.h>
#include <filesystem>

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

class MainWindow : public FileMenager, public OpenFile
{
    private:
        int width;
        int height;
        PythonKernel* kernel = nullptr;

        std::vector<std::unique_ptr<BaseCell>> cells;

        std::map<std::string, std::string> dict2map(const std::string dict);

        void ShowDirectoryNode(const std::filesystem::directory_entry& entry, std::string& selected_path);
        void ShowDirectoryTree(const std::filesystem::path& path, std::string& selected_path);
        void HandleDragDropTarget(const std::filesystem::path& target);

    protected:

        std::filesystem::path selected_path{};

        ImVec4 codeEditor();
        ImVec4 varTable();
        ImVec4 menuBar();
        ImVec4 workSpace();

        void initKernel(std::string path);
        void stopKernel();
        void resetKernel();


    public:
        MainWindow();
        ~MainWindow();

        ImVec4 mainWindow();
        void refresDisp(ImVec2 dispSize);
};

class OpenFileBrowser {
public:
    OpenFileBrowser();
    void Open();
    void Render();
    void set_selectPath(std::filesystem::path& path);

private:
    ImGui::FileBrowser fileDialog;
    std::filesystem::path* path = nullptr;
    std::filesystem::path tempSelection;
    bool hasTempSelection = false;
};
#endif //MAIN_WINDOW_H
