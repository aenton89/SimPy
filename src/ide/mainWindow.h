//
// Created by patryk on 10.02.26.
//

#include "../tabs/TabModule.h"
#include "gui/code_editor/notebookTab.h"
#include "gui/ui/workSpace.h"

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

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
    NotebookTab* notebookTab = nullptr;

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

#endif //MAIN_WINDOW_H
