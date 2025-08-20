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
#include "../data/structures.h"
#include "../data/blocks.h"
#include <implot.h>
#include <thread>
#include <algorithm>

const ImVec2 DEFAULT_DOCKED_RUN_SIZE = ImVec2(125, 85);
const ImVec2 DEFAULT_DOCKED_MENU_SIZE = ImVec2(150, 220);
const ImVec2 DEFAULT_UNDOCKED_RUN_SIZE = ImVec2(125, 85);
const ImVec2 DEFAULT_UNDOCKED_MENU_SIZE = ImVec2(150, 220);

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
        ImVec2 undockedSize = ImVec2(200, 150);
        bool isDocked = false;
    };

private:
    void drawBox(Block& box);
    void drawConnections();
    void drawStartButton();
    void drawMenu();
    void zoom();
    void applyCanvasTransform();

    // funkcje pomocnicze dla dockingu
    ImVec2 calculateDockedPosition(DockPosition position, ImVec2 size, bool isStartWindow = false);
    ImVec2 calculateDockedSize(DockPosition position, bool isStartWindow = false);
    DockPosition checkDockPosition(ImVec2 windowPos, ImVec2 windowSize);


    int next_id = 0;
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
    // odległość od krawędzi do snap'owania
    float dockSnapDistance = 50.0f;

    Model model;
};

#endif //GUICLASS_H