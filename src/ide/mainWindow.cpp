//
// Created by patryk on 10.02.26.
//

#include "mainWindow.h"



MainWindow::MainWindow() : TabModule("Ide") {
    this->workspace = new workSpace();
    this->notebookTab = new NotebookTile("/home/patryk/CLionProjects/SimPy/src/ide/core/SimPy_venv/SimPy_venv/bin/python3");

}

void MainWindow::menuBarFile() {
    if (ImGui::MenuItem("New", "Ctrl+N")) {}
    if (ImGui::MenuItem("Open file", "Ctrl+O")) {}
    if (ImGui::MenuItem("Open Folder", "Ctrl+O+F")) {}
    if (ImGui::MenuItem("Save", "Ctrl+S", false)) {}
    if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {}
    if (ImGui::MenuItem("Exit", "Ctrl+W")) {}
}

void MainWindow::menuBarEdit() {
    if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* akcja */ }
    if (ImGui::MenuItem("Redo", "Ctrl+Y")) { /* akcja */ }
}

void MainWindow::menuBarSettings() {
    // tu zaminic na taka ladna kalse jakl jest w blueprint
    ImGui::MenuItem("Light mode", "Ctrl+L", false);
}

void MainWindow::update() {
    TabModule::update();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float menuBarHeight = ImGui::GetFrameHeight();
    // cienki pasek
    float topBarHeight = 50.0f;
    float sidebarWidth = 30.0f;

    float wsWidth = 200.0f;

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    // GÓRNY PASEK
    ImGui::SetNextWindowPos(ImVec2(
        0,
        viewport->Pos.y + menuBarHeight
    ));

    ImGui::SetNextWindowSize(ImVec2(
        viewport->Size.x,
        topBarHeight
    ));

    ImGui::SetNextWindowBgAlpha(0.8f); // taki sam jak sidebar

    if (ImGui::Begin("TopBar", nullptr, window_flags)) {
        // Content kotry bedzie na tym pasku
    }
    ImGui::End();

    // SIDEBAR
    ImGui::SetNextWindowPos(ImVec2(
        0,
        topBarHeight + menuBarHeight
    ));

    ImGui::SetNextWindowSize(ImVec2(
        sidebarWidth,
        viewport->Size.y - topBarHeight - menuBarHeight
    ));

    // ten sam alpha
    ImGui::SetNextWindowBgAlpha(0.8f);

    if (ImGui::Begin("Sidebar", nullptr, window_flags)) {
        ImVec2 buttonSize(20, 20);
        float xCenter = (ImGui::GetWindowSize().x - buttonSize.x) / 2.0f;
        float yCenter = 10.0f;

        ImGui::SetCursorPos(ImVec2(xCenter, yCenter));

        if (ImGui::Button("F", buttonSize)) {
            showWorkspace = !showWorkspace;
        }
    }

    ImGui::End();



    // Notebook
    //if (ImGui::Begin("Notebook", nullptr, window_flags)) {
        if (showWorkspace) {
            this->notebookTab->SetPos(ImVec2(
                sidebarWidth + wsWidth,
                menuBarHeight + topBarHeight
            ));
            this->notebookTab->SetSize(ImVec2(
                viewport->Size.x - sidebarWidth - wsWidth,
                viewport->Size.y - menuBarHeight - topBarHeight
            ));
        } else {
            this->notebookTab->SetPos(ImVec2(
                sidebarWidth,
                menuBarHeight + topBarHeight
            ));
            this->notebookTab->SetSize(ImVec2(
                viewport->Size.x - sidebarWidth,
                viewport->Size.y - menuBarHeight - topBarHeight
            ));
        }
    //}
    //ImGui::End();


    // WORKSPACE
    if (showWorkspace && workspace) {

        ImVec2 wsPos(
            sidebarWidth,
            menuBarHeight + topBarHeight
        );

        ImVec2 wsSize(
            wsWidth,
            viewport->Size.y - menuBarHeight - topBarHeight
        );

        workspace->Render(wsPos, wsSize);
    }
    notebookTab->Draw();
}


