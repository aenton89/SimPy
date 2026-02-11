//
// Created by patryk on 10.02.26.
//

#include "mainWindow.h"

MainWindow::MainWindow() : TabModule("Ide") {

}


void MainWindow::menuBarFile() {
    if (ImGui::MenuItem("New", "Ctrl+N")) {}

    if (ImGui::MenuItem("Open", "Ctrl+O")) {}

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

}
