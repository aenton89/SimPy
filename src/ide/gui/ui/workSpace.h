//
// Created by patryk on 11.02.26.
//

#include <fstream>
#include <iostream>
#include "../../core/file_manipulation/FileSystemService.h"

#ifndef WORKSPACEMODULE_H
#define WORKSPACEMODULE_H

class workSpace
{
public:
    workSpace();
    ~workSpace();

    void Update();
    void Render(ImVec2 pos_xy, ImVec2 window_size);

    void set_ProjectPath(const fs::path& path);
    fs::path get_path2open();
    ImVec2 GetWindowSize();
    ImVec2 GetWindowPosition();

    fs::path project_path;
    fs::path open_path;

protected:
    void OpenContextMenu(const fs::directory_entry& entry);
    void ShowDirectoryNode(const fs::directory_entry& entry, std::string& selected_path);
    void ShowDirectoryTree(const fs::path& path, std::string& selected_path);
    void HandleDragDropTarget(const fs::path& target);

private:
    FileSystemService fsService;
};

#endif //WORKSPACEMODULE_H
