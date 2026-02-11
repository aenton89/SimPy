//
// Created by tajbe on 30.12.2025.
//
#pragma once
#include "TabModule.h"
#include <imgui_impl_glfw.h>

class GUICore;

class MainWindow;
/*
 * manager zarządzający zakładkami w aplikacji
 * przechowuje moduły, obsługuje przełączanie między nimi i renderowanie paska zakładek
 */
class TabManager {
private:
    // TODO: wektor wskaźników do zakładek
    std::vector<std::unique_ptr<TabModule>> tabs;
    // TODO: indeks aktualnie otwartej zakładki
    int currentTab = -1;

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

    // TODO: lista z wyborem zakładek na górze, zamykanie i otwieranie zakładek (narazie defaultowo otwiera GUICore)
    void changeTab(int index);
    void closeTab(int index);
    template<typename T = GUICore>
    void openTab();

    // TODO: serializacja jakie zakładki są otwarte - ALE NARAZIE BEZ TEGO
};