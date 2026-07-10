//
// Created by tajbe on 30.12.2025.
//
#pragma once
#include "TabModule.h"
#include <imgui_impl_glfw.h>
#include "../workspace/workSpace.h"

class BluePrintTab;

class NotebookTab;
/*
 * manager zarządzający zakładkami w aplikacji
 * przechowuje moduły, obsługuje przełączanie między nimi i renderowanie paska zakładek
 */

struct ActiveArea {
    ImVec2 pos;
    ImVec2 size;
};

class TabManager {
private:
    // TODO: wektor wskaźników do zakładek
    std::vector<std::unique_ptr<TabModule>> tabs;
    // TODO: indeks aktualnie otwartej zakładki
    int currentTab = -1;

    workSpace* workspace = nullptr;
    ImGuiViewport* viewport; // imo trzeba dodac to jako statyczne pole albo global ale narazie zostawie to tak

    bool showWorkspace = false;

protected:
    FileManager fileManager;


public:
    // TODO: rzeczy zwiazane z renderowaniem
    GLFWwindow* window = nullptr;
    const char* glsl_version = nullptr;

    TabManager();
    ~TabManager() = default;

    // TODO: dodać updateOpen() - aktualizacja tylko używanej zakładki
    void updateOpen();

    // TODO: przenieść render(), shutdown(), newframe() i część init()
    void init(GLFWwindow* win, const char* version);
    void newFrame();
    void render();
    void shutdown();
    // TODO: swoje update() - wywołuje updateOpen() i wykonuje własne UI:
    void update();

    // TODO: lista z wyborem zakładek na górze, zamykanie i otwieranie zakładek (narazie defaultowo otwiera blueprintTab)
    void changeTab(int index);
    void closeTab(int index);
    template<typename T = BluePrintTab>
    void openTab();

    // Metody uzywane do wyetlania side bara i workspaca. Mozna to chycba potem porpstu przneisc do update ale tak jest imo czytleniej
    void drawWorkspace();

    // to jest potrzbne po to zeby gdy workspace jest otearty zeby zawrtosc w klasach klasy TabModule sie tez przesuwaly
    ActiveArea getActiveArea();

    // TODO: serializacja jakie zakładki są otwarte - ALE NARAZIE BEZ TEGO

    // Moje podejscie do tych tabow
    template<typename T>
    T* getCurrentTabAs();
};