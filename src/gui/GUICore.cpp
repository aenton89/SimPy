//
// Created by tajbe on 18.04.2025.
//
#include <functional>
#include <iostream>
#include <thread>
#include <filesystem>
#include <thread>
#include <chrono>
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GUICore.h"
#include "../core/structures/BasicBlock.h"
#include "math/solvers/SolverMethod.h"
#include "ui/UIStyles.h"

CEREAL_FORCE_DYNAMIC_INIT(blocks)



GUICore::GUICore() {
    dockingManager.setGUICore(this);
    fileManager.setGUICore(this);
    viewportManager.setGUICore(this);
    blocksManager.setGUICore(this);
    connectionManager.setGUICore(this);
}

// initialization of evertything regarding ImGui
void GUICore::init(GLFWwindow* win, const char* version) {
    window = win;
    glsl_version = version;

    // jakaś defaultowa ikonka, potem zmienie
    GLFWimage images[1];
    std::string iconPath = std::string(ASSETS_DIR) + "/app_icons/icon_v3.png";
    images[0].pixels = stbi_load(iconPath.c_str(), &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
    if (!images[0].pixels) {
        std::cerr << "ERROR: couldn't load icon.png!" << std::endl;
    }

    Model::timeStep = 0.01;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImPlot::CreateContext();

    // setup platform/ renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // optional: setup style and custom colors
    ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 gray = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

    style.Colors[ImGuiCol_WindowBg] = gray;
    style.Colors[ImGuiCol_ChildBg] = gray;
    style.Colors[ImGuiCol_PopupBg]= gray;
}

void GUICore::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUICore::update() {
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

    // rysowanie paska na górze
    drawMenuBar();

    // tryb jasny/ciemny bleh
    viewportManager.lightMode();

    // rysowanie bloczków
    for (auto& block : model.getBlocks()) {
        if (block->open)
            blocksManager.drawBlock(*block);
    }

    // rysowanie połączeń między bloczkami
    connectionManager.drawConnections();

    // obsługa viewportu: zoom i przesuwanie
    viewportManager.zoomAndPanning();

    // zbieramy ID blocków do usunięcia
    std::vector<int> to_remove;
    for (auto& box : model.getBlocks()) {
        if (!box->open) {
            to_remove.push_back(box->id);
        }
    }

    // usuwamy połączenia do/z tych boxów
    for (auto& box : model.getBlocks()) {
        auto& conns = box->connections;
        conns.erase(std::remove_if(conns.begin(), conns.end(),
            [&](int id) {
                return std::find(to_remove.begin(), to_remove.end(), id) != to_remove.end();
            }), conns.end());
    }

    // usuwamy same boxy
    model.getBlocks().erase(std::remove_if(model.getBlocks().begin(), model.getBlocks().end(),
        [](const std::shared_ptr<Block>& box) {
            return !box->open;
        }), model.getBlocks().end());

    // MAMMA MIA last time i forgor about de grid
    viewportManager.drawGrid();
}

void GUICore::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUICore::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();
}

void GUICore::drawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
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
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* akcja */ }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) { /* akcja */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            ImGui::MenuItem("Light mode", "Ctrl+L", &uiPreferences.lightMode);
            ImGui::MenuItem("Show grid", "Ctrl+G", &uiPreferences.gridEnabled);

            if (ImGui::BeginMenu("Grid Settings")) {
                ImGui::SliderFloat("Grid Spacing", &uiPreferences.gridSpacing, 10.0f, 200.0f);
                ImGui::SliderFloat("Line Thickness", &uiPreferences.gridThickness, 0.5f, 5.0f);

                // color picker for grid
                ImVec4 grid_color = ImGui::ColorConvertU32ToFloat4(uiPreferences.gridColor);
                if (ImGui::ColorEdit4("Grid Color", (float*)&grid_color, ImGuiColorEditFlags_AlphaPreview))
                    uiPreferences.gridColor = ImGui::ColorConvertFloat4ToU32(grid_color);

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
