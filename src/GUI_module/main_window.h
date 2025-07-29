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

    protected:

        ImVec4 codeEditor();
        ImVec4 varTable();
        ImVec4 menuBar();
        ImVec4 workSpace();

        void initKernel(std::string path);
        void stopKernel();
        void resetKernel();

        //std::string OpenPopup(const std::string& textTitle);
        //void OpenFileBrowser(std::string& selectedPath);


    public:
        MainWindow();
        ~MainWindow();

        ImVec4 mainWindow();
        void refresDisp(ImVec2 dispSize);
};

class OpenFileBrowser
{
    public:
        OpenFileBrowser();

        void Open();
        void Render(std::string& selectedPath);

    private:
        ImGui::FileBrowser fileDialog;
};
#endif //MAIN_WINDOW_H
