//
// Created by tajbe on 24.10.2025.
//
#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <thread>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <portable-file-dialogs.h>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/atomic.hpp>
#include <cereal/archives/xml.hpp>
#include "../core/structures/BasicBlock.h"
#include "../core/structures/Model.h"
#include "../core/structures/Connection.h"
#include "../core/structures/Blocks.h"
#include "windows/DockableWindowManager.h"
#include "files/FileManager.h"
#include "viewport/ViewportManager.h"
#include "ui/UIPreferences.h"
#include "windows/BlocksManager.h"
#include "viewport/ConnectionManager.h"



/*
 * main logic core for UI is here
 * combines all modules together
 */
class GUICore {
public:
    GUICore();

    void init(GLFWwindow* window, const char* glsl_version);
    void newFrame();
    virtual void update();
    void render();
    void shutdown();

    // awkward ale chyba tu zostanie xddd
    void drawMenuBar();

    // nowe manager'y
    DockableWindowManager dockingManager;
    FileManager fileManager;
    ViewportManager viewportManager;
    BlocksManager blocksManager;
    ConnectionManager connectionManager;
    // preferencje ui
    UIPreferences uiPreferences;

    GLFWwindow* window = nullptr;
    const char* glsl_version = nullptr;
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