//
// Created by patryk on 11.02.26.
//
#pragma once

#include <fstream>
#include <iostream>
#include "file_manipulation/FileSystemService.h"
#include "../tabs/TabManager.h"
#include "../files/FileManager.h"
#include "portable-file-dialogs.h"
#include "../blueprints/gui/BluePrintTab.h"
#include "../ide/NotebookTab.h"


class workSpace {
public:
    workSpace(TabManager* tabManager) : tabManager(tabManager) {};
    ~workSpace();
    
    void Update();
    void Render(ImVec2 pos_xy, ImVec2 window_size);

    void set_ProjectPath(const fs::path& path);
    fs::path get_path2open();
    ImVec2 GetWindowSize();
    ImVec2 GetWindowPosition();

protected:
    void OpenContextMenu(const fs::directory_entry& entry);
    void ShowDirectoryNode(const fs::directory_entry& entry, std::string& selected_path);
    void ShowDirectoryTree(const fs::path& path, std::string& selected_path);
    void HandleDragDropTarget(const fs::path& target);

private:
    FileSystemService fsService;
    fs::path projectPath;
    fs::path openPath;
    TabManager* tabManager = nullptr;

};
