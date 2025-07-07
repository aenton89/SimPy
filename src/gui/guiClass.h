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

    // TODO: łączenie w całość
    // bool getisRunning();
    // void setIsRunning(bool start);
    // guiClass(Model* _model) : model(_model){};


private:
    // struct Box {
    //     int id;
    //     ImVec2 position = ImVec2(100, 100);
    //     ImVec2 size = ImVec2(300, 120);
    //     bool open = true;
    //     std::vector<int> connections;
    //
    //     virtual ~Box() = default;
    //     virtual void drawContent() = 0;
    // };
    //
    // struct FloatBox : Box {
    //     float value = 0.0f;
    //     FloatBox(int i) { id = i; }
    //
    //     void drawContent() override {
    //         ImGui::SliderFloat("Value", &value, 0.0f, 100.0f);
    //     };
    // };
    //
    // struct StringBox : Box {
    //     char text[128] = "Type here";
    //     StringBox(int i) { id = i; }
    //
    //     void drawContent() override {
    //         ImGui::InputText("Text", text, IM_ARRAYSIZE(text));
    //     }
    // };

    // TODO: łączenie w całość
    void drawBox(Block& box);
    void drawConnections();
    void drawStartButton();

    int next_id = 0;
    // std::vector<std::unique_ptr<Box>> boxes;
    std::optional<int> dragging_from;
    GLFWwindow* window = nullptr;
    const char* glsl_version = nullptr;
    // id boxów, do których są połączenia
    // std::optional<std::pair<int, int>> dragging_connection;

    // TODO: łączenie w całość
    bool isRunning = false;
    Model model;
};

#endif //GUICLASS_H