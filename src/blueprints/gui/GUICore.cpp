//
// Created by tajbe on 18.04.2025.
//
#include "GUICore.h"
#include <imgui_impl_opengl3.h>
#include "GLFW/glfw3.h"
#include <functional>
#include <thread>
#include <chrono>
#include "../core/structures/BasicBlock.h"
// #include <implot.h>

CEREAL_FORCE_DYNAMIC_INIT(blocks)



GUICore::GUICore() : TabModule("Blueprints"){
    dockingManager.setGUICore(this);
    fileManager.setGUICore(this);
    viewportManager.setGUICore(this);
    blocksManager.setGUICore(this);
    connectionManager.setGUICore(this);

    // TODO: narazie wstawiam init() do konstruktora
    Model::timeStep = 0.01;
}

void GUICore::update() {
    TabModule::update();

    ImGuiIO& io = ImGui::GetIO();

    // logika zaznaczania/odznaczania boxów
    blocksManager.clearSelectedBlocks(io);
    blocksManager.duplicateSelectedBlocks(io);
    blocksManager.deleteSelectedBlocks(io);
    blocksManager.selectAllBlocks(io);

    // skróty klawiszowe
    viewportManager.turnLightModeOnShortcut(io);
    viewportManager.turnGridOnShortcut(io);
    fileManager.saveStateShortcut(io);
    fileManager.loadStateShortcut(io);
    fileManager.exitFileShortcut(io);
    fileManager.newFileShortcut(io);

    // rysowanie okien, które się dockują
    dockingManager.drawMenu();
    dockingManager.drawStartButton();

    // tryb jasny/ciemny bleh
    viewportManager.lightMode();

    // rysowanie bloczków
    for (auto& block : model.getBlocks()) {
        if (block->open)
            blocksManager.drawBlock(block);
    }

    // rysowanie połączeń między bloczkami
    connectionManager.drawConnections();

    // obsługa viewportu: zoom i przesuwanie
    viewportManager.zoomAndPanning();

    // zbieramy ID blocków do usunięcia
    std::vector<int> to_remove;
    for (auto& box : model.getBlocks()) {
        if (!box->open)
            to_remove.push_back(box->id);
    }

    // usuwamy połączenia do/z tych boxów
    for (int id : to_remove) {
        if (auto blockToRemove = model.findBlockById(id))
            model.removeAllConnectionsForBlock(blockToRemove);
    }

    // usuwamy same boxy
    std::erase_if(model.getBlocks(), [](const std::shared_ptr<Block>& box) {
                      return !box->open;
                  });

    // MAMMA MIA last time i forgor about de grid
    viewportManager.drawGrid();
}

void GUICore::menuBarFile() {
    if (ImGui::MenuItem("New", "Ctrl+N"))
        fileManager.newFile();
    if (ImGui::MenuItem("Open", "Ctrl+O"))
        fileManager.openFileDialog();
    if (ImGui::MenuItem("Save", "Ctrl+S", false, !fileManager.currentFilePath.empty() || fileManager.hasUnsavedChanges))
        fileManager.saveFile();
    if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
        fileManager.saveFileDialog();
    if (ImGui::MenuItem("Exit", "Ctrl+W"))
        fileManager.exitFile();
}

void GUICore::menuBarEdit() {
    if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* akcja */ }
    if (ImGui::MenuItem("Redo", "Ctrl+Y")) { /* akcja */ }
}

void GUICore::menuBarSettings() {
    ImGui::MenuItem("Light mode", "Ctrl+L", &uiPreferences.lightMode);

    if (ImGui::BeginMenu("Connection Path Type")) {
        if (ImGui::Selectable("Orthogonal Path", connectionManager.useOrthogonalLines))
            connectionManager.useOrthogonalLines = true;
        if (ImGui::Selectable("Curve Path", !connectionManager.useOrthogonalLines))
            connectionManager.useOrthogonalLines = false;

        ImGui::EndMenu();
    }

    ImGui::MenuItem("Show grid", "Ctrl+G", &uiPreferences.gridEnabled);

    if (ImGui::BeginMenu("Grid Settings")) {
        ImGui::SliderFloat("Grid Spacing", &uiPreferences.gridSpacing, 10.0f, 200.0f);
        ImGui::SliderFloat("Line Thickness", &uiPreferences.gridThickness, 0.5f, 5.0f);

        // color picker for grid
        ImVec4 grid_color = ImGui::ColorConvertU32ToFloat4(uiPreferences.gridColor);
        if (ImGui::ColorEdit4("Grid Color", reinterpret_cast<float *>(&grid_color), ImGuiColorEditFlags_AlphaPreview))
            uiPreferences.gridColor = ImGui::ColorConvertFloat4ToU32(grid_color);

        ImGui::EndMenu();
    }
}