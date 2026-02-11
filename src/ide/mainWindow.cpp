//
// Created by patryk on 10.02.26.
//

#include "mainWindow.h"

MainWindow::MainWindow() : TabModule("Ide") {
    this->workspace = new workSpace();
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
    ImGui::MenuItem("Light mode", "Ctrl+L", false); // tu zaminic na taka ladna kalse jakl jest w blueprint
}

void MainWindow::update() {
    TabModule::update();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float menuBarHeight = ImGui::GetFrameHeight();
    float sidebarWidth = 30.0f;

    // ===================== Pasek boczny =====================
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(sidebarWidth, viewport->Size.y - menuBarHeight));
    ImGui::SetNextWindowBgAlpha(0.8f);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("Sidebar", nullptr, window_flags);

    // Wyśrodkowany kwadratowy guzik
    ImVec2 buttonSize(20, 20);
    float xCenter = (ImGui::GetWindowSize().x - buttonSize.x) / 2.0f;
    float yCenter = 10.0f; // trochę odstępu od góry paska

    ImGui::SetCursorPos(ImVec2(xCenter, yCenter));

    if (ImGui::Button("F", buttonSize)) { // np. ikona folderu
        showWorkspace = !showWorkspace;   // przełączamy widoczność workspace
    }

    ImGui::End();

    // ===================== Workspace =====================
    if (showWorkspace && workspace) {
        ImVec2 wsPos(30, menuBarHeight);          // pozycja okna workspace
        ImVec2 wsSize(300, viewport->Size.y - menuBarHeight);
        workspace->Render(wsPos, wsSize);
    }
}


