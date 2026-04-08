//
// Created by patryk on 10.02.26.
//
#pragma once

#include "../tabs/TabModule.h"
#include "gui/code_editor/notebookTile.h"
#include "gui/ui/workSpace.h"



class MainWindow : public TabModule {
public:
    MainWindow();

    void update() override;
    // awkward ale chyba tu zostanie xddd
    void menuBarFile() override;
    void menuBarEdit() override;
    void menuBarSettings() override;

protected:
    workSpace* workspace = nullptr;
    NotebookTile* notebookTab = nullptr;

    bool showWorkspace = false;

    // preferencje ui
    // UIPreferences uiPreferences; - tu dodac te kolorki
public:
    /* NIE jest serializowane:
    - wskaźniki systemowe: window, glsl_version - to musi być zainicjalizowane przez system
    - stany runtime: simulationRunning, canvasDragging, isDraggingWindow - to są stany tymczasowe
    - pozycje myszy/drag: dragStartPos, windowDragOffset - bo to są stany tymczasowe
    - dragging_from: reset przy ładowaniu
     */
};
