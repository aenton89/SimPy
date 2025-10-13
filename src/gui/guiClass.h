//
// Created by tajbe on 18.04.2025.
//
#ifndef GUICLASS_H
#define GUICLASS_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <implot.h>
#include <thread>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/atomic.hpp>
#include "../data/structures.h"
#include "../data/blocks.h"



const ImVec2 DEFAULT_DOCKED_START_SIZE = ImVec2(195, 180);
const ImVec2 DEFAULT_DOCKED_MENU_SIZE = ImVec2(245, 200);

enum DockableWindowType {
    Menu,
    Start
};



static float LengthSqr(const ImVec2& a, const ImVec2& b) {
    ImVec2 d = ImVec2(a.x - b.x, a.y - b.y);
    return d.x * d.x + d.y * d.y;
}

static ImVec2 BezierCubicCalc(const ImVec2& p1, const ImVec2& c1, const ImVec2& c2, const ImVec2& p2, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    ImVec2 result = ImVec2(0, 0);
    result.x = uuu * p1.x;
    result.x += 3 * uu * t * c1.x;
    result.x += 3 * u * tt * c2.x;
    result.x += ttt * p2.x;

    result.y = uuu * p1.y;
    result.y += 3 * uu * t * c1.y;
    result.y += 3 * u * tt * c2.y;
    result.y += ttt * p2.y;

    return result;
}


class guiClass {
public:
    void init(GLFWwindow* window, const char* glsl_version);
    void newFrame();
    virtual void update();
    void render();
    void shutdown();

    // dockowanie menu i start
    enum class DockPosition {
        None,
        Left,
        Right,
        Top,
        Bottom
    };

    struct DockableWindow {
        DockPosition position = DockPosition::None;
        ImVec2 undockedPos = ImVec2(100, 100);
        ImVec2 dockedPos = ImVec2(100, 100);
        bool isDocked = false;

        template<class Archive>
        void serialize(Archive& ar) {
            int pos = static_cast<int>(position);
            ar(CEREAL_NVP(pos),
               CEREAL_NVP(isDocked));

            // serializacja ImVec2 jako vector<float>
            std::vector<float> undocked = {undockedPos.x, undockedPos.y};
            std::vector<float> docked = {dockedPos.x, dockedPos.y};
            ar(cereal::make_nvp("undockedPos", undocked),
               cereal::make_nvp("dockedPos", docked));

            if constexpr (Archive::is_loading::value) {
                position = static_cast<DockPosition>(pos);
                undockedPos = ImVec2(undocked[0], undocked[1]);
                dockedPos = ImVec2(docked[0], docked[1]);
            }
        }
    };

private:
    void drawBox(Block& box);
    void drawConnections();
    void drawStartButton();
    void drawMenu();
    void drawMenuBar();
    void drawGrid();
    void noLightMode();
    void zoom();
    void applyCanvasTransform();

    // funkcje pomocnicze dla dockingu
    ImVec2 calculateDockedPosition(DockPosition position, DockableWindowType windowType);
    ImVec2 calculateDockedSize(DockPosition position, DockableWindowType windowType);
    DockPosition checkDockPosition(ImVec2 windowPos, ImVec2 windowSize);

    // funkcje używane w update() do zaznaczania/odznaczania box'ów
    void clearSelectedBlocks(const ImGuiIO& io);
    void duplicateSelectedBlocks(const ImGuiIO& io);
    void deleteSelectedBlocks(const ImGuiIO& io);
    void selectAllBlocks(const ImGuiIO& io);

    // skróty klawiszowe
    void turnLightModeOn(const ImGuiIO& io);
    void turnGridOn(const ImGuiIO& io);

    std::optional<int> dragging_from;
    GLFWwindow* window = nullptr;
    const char* glsl_version = nullptr;
    // odpowiada za simulate i zapewnienie, że nie odpali sie drugi raz w trakcie trwającej symulacji
    std::atomic<bool> simulationRunning = false;

    // zoomowanie
    float zoomAmount = 1.0f;
    ImVec2 viewOffset = ImVec2(0.0, 0.0);
    float zoomSpeed = 0.1f;
    // przesuwanie canvas'u
    bool canvasDragging = false;
    ImVec2 dragStartPos = ImVec2(0.0, 0.0);
    ImVec2 dragStartOffset = ImVec2(0.0, 0.0);
    // dragging vs zooming -> problem który fix'ujemy
    bool isDraggingWindow = false;
    int draggedWindowId = -1;
    // offset dla przeciągania okna
    ImVec2 windowDragOffset;

    // zmienne dla dockingu
    DockableWindow menuWindow;
    DockableWindow startWindow;
    DockableWindow settingsWindow;
    std::vector<DockableWindow> dockedWindows;
    // odległość od krawędzi do snap'owania
    float dockSnapDistance = 50.0f;

    // zminne odpwoedzialne za symualcje (czas symulacji, sampling time, rodzaj solvera itd.)
    std::string solverName = "RK4";
    std::string solverPrecison;

    bool lightMode = false;

    // grid settings
    bool gridEnabled = true;
    float gridSpacing = 50.0f;
    float gridThickness = 1.0f;
    ImU32 gridColor = IM_COL32(0, 0, 0, 100);

    // dla zaznaczania wielu box'ów
    std::set<int> selectedBlocks;
    bool isMultiSelectMode = false;
    // grupowe przeciąganie zaznaczonych boxów
    bool isGroupDragging = false;
    ImVec2 groupDragStartMousePos;
    std::unordered_map<int, ImVec2> groupInitialPositions;


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
        // podstawowe dane stanu
        // ar(CEREAL_NVP(next_id));

        // parametry zoom i view
        ar(CEREAL_NVP(zoomAmount),
           CEREAL_NVP(zoomSpeed));

        // serializacja ImVec2 viewOffset
        std::vector<float> offset = {viewOffset.x, viewOffset.y};
        ar(cereal::make_nvp("viewOffset", offset));

        // docking windows
        ar(CEREAL_NVP(menuWindow),
           CEREAL_NVP(startWindow),
           CEREAL_NVP(settingsWindow),
           CEREAL_NVP(dockedWindows),
           CEREAL_NVP(dockSnapDistance));

        // solver settings
        ar(CEREAL_NVP(solverName));

        // UI preferences
        ar(CEREAL_NVP(lightMode),
           CEREAL_NVP(gridEnabled),
           CEREAL_NVP(gridSpacing),
           CEREAL_NVP(gridThickness));

        // grid color jako uint32
        uint32_t gridCol = gridColor;
        ar(cereal::make_nvp("gridColor", gridCol));

        // selection state
        ar(CEREAL_NVP(selectedBlocks),
           CEREAL_NVP(isMultiSelectMode));

        // model
        ar(CEREAL_NVP(model));

        if constexpr (Archive::is_loading::value) {
            viewOffset = ImVec2(offset[0], offset[1]);
            gridColor = static_cast<ImU32>(gridCol);

            // Reset runtime state
            simulationRunning = false;
            canvasDragging = false;
            isDraggingWindow = false;
            draggedWindowId = -1;
            isGroupDragging = false;
            dragging_from.reset();
            groupInitialPositions.clear();
        }
    }
};

#endif //GUICLASS_H