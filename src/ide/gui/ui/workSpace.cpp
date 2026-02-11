//
// Created by patryk on 11.02.26.
//

#include "workSpace.h"
#include <iostream>
#include <fstream>

workSpace::workSpace() {
    this->project_path = fs::current_path();
}

workSpace::~workSpace() { }

void workSpace::Render(ImVec2 pos, ImVec2 size)
{
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);

    ImGui::Begin("Workspace", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    ImGui::Text("Work Space");

    static std::string path;

    if (!project_path.empty() && ImGui::TreeNode(this->project_path.filename().string().c_str()))
    {
        ShowDirectoryTree(this->project_path, path);

        ImGui::TreePop();
    }

    ImGui::End();
}

void workSpace::Update() { }

ImVec2 workSpace::GetWindowSize() { return ImGui::GetWindowSize(); }
ImVec2 workSpace::GetWindowPosition() { return ImGui::GetWindowPos(); }

void workSpace::set_ProjectPath(const fs::path& path) { this->project_path = path; }
fs::path workSpace::get_path2open() { return this->open_path; }

///////////////////////////////////////////////////// Metody związane z operacjami na drzewku projektu /////////////////////////////////////////////

void workSpace::OpenContextMenu(const fs::directory_entry& entry)
{
    if (!ImGui::BeginPopupContextItem())
        return;

    ImGui::Text("File: %s", entry.path().filename().string().c_str());
    ImGui::Separator();

    if (ImGui::MenuItem("Open"))
        this->open_path = entry.path();

    if (ImGui::MenuItem("Copy abs path"))
        ImGui::SetClipboardText(entry.path().c_str());

    static char nameBuffer[256] = "";

    if (ImGui::BeginMenu("Create"))
    {
        ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));

        if (ImGui::MenuItem("Create folder") && nameBuffer[0] != '\0')
        {
            if (!fsService.CreateFolder(entry.path(), nameBuffer))
                std::cerr << "Nie udało się stworzyć folderu\n";
            memset(nameBuffer, 0, sizeof(nameBuffer));
        }

        if (ImGui::MenuItem("Create file") && nameBuffer[0] != '\0')
        {
            if (!fsService.CreateFile(entry.path(), nameBuffer))
                std::cerr << "Nie udało się stworzyć pliku\n";
            memset(nameBuffer, 0, sizeof(nameBuffer));
        }

        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Remove"))
    {
        if (!fsService.Delete(entry.path()))
            std::cerr << "Nie udało się usunąć pliku/folderu\n";
    }

    if (ImGui::MenuItem("Close"))
        ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}

void workSpace::HandleDragDropTarget(const fs::path& target)
{
    if (!ImGui::BeginDragDropTarget())
        return;

    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
    {
        const char* sourcePath = (const char*)payload->Data;
        fs::path source(sourcePath);
        fs::path destination = target / source.filename();

        if (fs::exists(destination))
            destination = target / (source.filename().string() + "_copy");

        if (!fsService.Move(source, destination))
            std::cerr << "Nie udało się przenieść pliku/folderu\n";
    }

    ImGui::EndDragDropTarget();
}

void workSpace::ShowDirectoryNode(const fs::directory_entry& entry, std::string& selected_path)
{
    const std::string name = entry.path().filename().string();
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (entry.is_directory())
    {
        bool opened = ImGui::TreeNodeEx(name.c_str(), nodeFlags);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            selected_path = entry.path().string();

        OpenContextMenu(entry);

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            std::string folderPath = entry.path().string();
            ImGui::SetDragDropPayload("FILE_PATH", folderPath.c_str(), folderPath.size() + 1);
            ImGui::Text("Przenoszę folder: %s", name.c_str());
            ImGui::EndDragDropSource();
        }

        HandleDragDropTarget(entry.path());

        if (opened)
        {
            std::error_code ec;
            if (fs::exists(entry.path(), ec))
            {
                for (const auto& child : fs::directory_iterator(entry.path()))
                {
                    if (ec) break;
                    ShowDirectoryNode(child, selected_path);
                }
            }
            ImGui::TreePop();
        }
    }
    else
    {
        if (ImGui::Selectable(name.c_str()))
            selected_path = entry.path().string();

        OpenContextMenu(entry);

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            std::string filePath = entry.path().string();
            ImGui::SetDragDropPayload("FILE_PATH", filePath.c_str(), filePath.size() + 1);
            ImGui::Text("Przenoszę plik: %s", name.c_str());
            ImGui::EndDragDropSource();
        }
    }
}

void workSpace::ShowDirectoryTree(const fs::path& path, std::string& selected_path)
{
    if (!fs::exists(path) || !fs::is_directory(path)) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Folder doesn't exist: %s", path.string().c_str());
        return;
    }

    for (const auto& entry : fs::directory_iterator(path)) {
        ShowDirectoryNode(entry, selected_path);
    }
}
