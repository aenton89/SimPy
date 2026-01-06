//
// Created by tajbe on 24.10.2025.
//
#pragma once

#include <portable-file-dialogs.h>
#include <cereal/types/atomic.hpp>
#include "../core/structures/Model.h"
#include "../core/structures/Blocks.h"
#include "ui/UIPreferences.h"
#include "managers/windows/DockableWindowManager.h"
#include "managers/files/FileManager.h"
#include "managers/viewport/ViewportManager.h"
#include "managers/windows/BlocksManager.h"
#include "managers/viewport/ConnectionManager.h"
#include "../tabs/TabModule.h"
// #include <imgui.h>
// #include <memory>
// #include <string>



/*
 * main logic core for UI is here
 * combines all modules together
 */
class GUICore : public TabModule {
public:
    GUICore();

    void update() override;
    // awkward ale chyba tu zostanie xddd
    void menuBarFile() override;
    void menuBarEdit() override;
    void menuBarSettings() override;

    // nowe manager'y
    DockableWindowManager dockingManager;
    FileManager fileManager;
    ViewportManager viewportManager;
    BlocksManager blocksManager;
    ConnectionManager connectionManager;
    // preferencje ui
    UIPreferences uiPreferences;

    // odpowiada za simulate i zapewnienie, że nie odpali sie drugi raz w trakcie trwającej symulacji
    std::atomic<bool> simulationRunning = false;

    // TODO: ej Patryk a po co to jest, bo idk czy gdzieś używasz
    // zminne odpwoedzialne za symualcje (czas symulacji, sampling time, rodzaj solvera itd.)
    std::string solverName = "RK4";
    std::string solverPrecison;

    Model model;

public:
    /* NIE jest serializowane:
    - wskaźniki systemowe: window, glsl_version - to musi być zainicjalizowane przez system
    - stany runtime: simulationRunning, canvasDragging, isDraggingWindow - to są stany tymczasowe
    - pozycje myszy/drag: dragStartPos, windowDragOffset - bo to są stany tymczasowe
    - dragging_from: reset przy ładowaniu
     */
    template<class Archive>
    void serialize(Archive& ar) {
        // managers
        ar(CEREAL_NVP(dockingManager));
        ar(CEREAL_NVP(fileManager));
        ar(CEREAL_NVP(viewportManager));
        ar(CEREAL_NVP(blocksManager));
        ar(CEREAL_NVP(connectionManager));

        // ui preferences
        ar(CEREAL_NVP(uiPreferences));

        // solver settings
        ar(CEREAL_NVP(solverName));
        ar(CEREAL_NVP(solverPrecison));

        // model
        ar(CEREAL_NVP(model));

        // reset runtime state
        if constexpr (Archive::is_loading::value) {
            simulationRunning = false;
        }
    }
};