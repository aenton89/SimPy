//
// Created by tajbe on 10.11.2025.
//
#include "BlocksManager.h"
#include "../GUICore.h"



void BlocksManager::setGUICore(GUICore *gui) {
	guiCore = gui;
}

// logika czyszczenia zaznaczeń
void BlocksManager::clearSelectedBlocks(const ImGuiIO &io) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyShift) {
        ImVec2 mousePos = io.MousePos;
        bool clickedOnAnyTitle = false;
        for (auto& b : guiCore->model.getBlocks()) {
            ImVec2 screen_pos = ImVec2(b->position.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x, b->position.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y);
            ImVec2 screen_size = ImVec2(b->size.x * guiCore->viewportManager.zoomAmount, b->size.y * guiCore->viewportManager.zoomAmount);
            ImVec2 title_min = screen_pos;
            ImVec2 title_max = ImVec2(screen_pos.x + screen_size.x, screen_pos.y + ImGui::GetFrameHeight());
            if (mousePos.x >= title_min.x && mousePos.x <= title_max.x && mousePos.y >= title_min.y && mousePos.y <= title_max.y) {
                clickedOnAnyTitle = true;
                break;
            }
        }
        if (!clickedOnAnyTitle) {
            // klik poza titlebarami -> odznacz wszystko
            selectedBlocks.clear();
        }
    }
}

// logika kopiowania bloczków przez CTRL+D
void BlocksManager::duplicateSelectedBlocks(const ImGuiIO &io) {
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D, false)) {
        if (!selectedBlocks.empty()) {
            std::set<int> newSelection;

            for (int id : selectedBlocks) {
                auto it = std::find_if(guiCore->model.getBlocks().begin(), guiCore->model.getBlocks().end(), [&](auto& b){ return b->id == id; });
                if (it != guiCore->model.getBlocks().end()) {
                    std::shared_ptr<Block> copy = (*it)->clone();
                    copy->id = guiCore->model.next_id++;
                    copy->position = ImVec2((*it)->position.x + 20, (*it)->position.y + 20);

                    newSelection.insert(copy->id);
                    guiCore->model.getBlocks().push_back(std::move(copy));
                }
            }

            // przenosimy zaznaczenie na nowo dodane bloczki
            selectedBlocks = newSelection;
        }
    }
}

// usuwanie bloczków przez DEL
void BlocksManager::deleteSelectedBlocks(const ImGuiIO &io) {
    if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
        if (!selectedBlocks.empty()) {
            auto& blocks = guiCore->model.getBlocks();
            blocks.erase(std::remove_if(blocks.begin(), blocks.end(),[&](auto& b) {return selectedBlocks.count(b->id) > 0;}), blocks.end());

            // wyczyść zaznaczenie
            selectedBlocks.clear();
        }
    }
}

// zaznaczanie wszystkich blocków przez CTRL+A
void BlocksManager::selectAllBlocks(const ImGuiIO& io) {
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false)) {
        selectedBlocks.clear();
        for (const auto& block : guiCore->model.getBlocks()) {
            selectedBlocks.insert(block->id);
        }
    }
}

void BlocksManager::drawBlock(Block& box) {
    std::string title = "Box #" + std::to_string(box.id);

    // oblicz transformowaną pozycję
    ImVec2 world_pos = box.position;
    ImVec2 screen_pos = ImVec2(
        world_pos.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
        world_pos.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
    );
    ImVec2 screen_size = ImVec2(
        box.size.x * guiCore->viewportManager.zoomAmount,
        box.size.y * guiCore->viewportManager.zoomAmount
    );

    // MOLTO IMPORTANTE: NIE ustawiaj pozycji jeśli okno jest przeciągane przez użytkownika
    if (!ImGui::IsWindowAppearing() && !isDraggingWindow)
        ImGui::SetNextWindowPos(screen_pos);
    // inne okna nadal powinny być pozycjonowane
    else if (isDraggingWindow && draggedWindowId != box.id)
        ImGui::SetNextWindowPos(screen_pos);

    ImGui::SetNextWindowSize(screen_size);

    // skaluj font
    ImGui::GetIO().FontGlobalScale = guiCore->viewportManager.zoomAmount;

    // jeśli box jest zaznaczony - ustaw niebieskie titlebary
    bool pushedSelectionStyle = false;
    if (selectedBlocks.count(box.id)) {
        // onegdaj niebieski, teraz takied śmiszne cuś
        ImVec4 selColor = ImVec4(0.39f, 0.78f, 0.92f, 0.77f);
        ImGui::PushStyleColor(ImGuiCol_TitleBg, selColor);
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, selColor);
        pushedSelectionStyle = true;
    }

    // flagi okna - pozwól na normalne przeciąganie
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

    if (!ImGui::Begin(title.c_str(), &box.open, flags)) {
        ImGui::GetIO().FontGlobalScale = 1.0f;
        // tbh już nawet nie pamiętam skąd to się wzieło
        if (pushedSelectionStyle)
            ImGui::PopStyleColor(2);
        ImGui::End();
        return;
    }

    // pozycje okna i titlebara
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 current_screen_pos = ImGui::GetWindowPos();
    ImVec2 current_screen_size = ImGui::GetWindowSize();
    ImVec2 title_bar_min = current_screen_pos;
    ImVec2 title_bar_max = ImVec2(current_screen_pos.x + current_screen_size.x, current_screen_pos.y + ImGui::GetFrameHeight());
    ImVec2 window_min = current_screen_pos;
    ImVec2 window_max = ImVec2(current_screen_pos.x + current_screen_size.x, current_screen_pos.y + current_screen_size.y);

    // SHIFT+LPM: toggle selection gdy klikniesz **wewnątrz boxa poza titlebarem**
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && io.KeyShift) {
        ImVec2 m = io.MousePos;
        // klik w obszar okna, ale poza titlebarem
        if (m.x >= window_min.x && m.x <= window_max.x && m.y >= window_min.y && m.y <= window_max.y &&
            !(m.x >= title_bar_min.x && m.x <= title_bar_max.x && m.y >= title_bar_min.y && m.y <= title_bar_max.y)) {
            // toggle zaznaczenia tego boxa
            if (selectedBlocks.count(box.id))
                selectedBlocks.erase(box.id);
            else
                selectedBlocks.insert(box.id);
        }
    }

    // klik w titlebar (LPM):
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 m = io.MousePos;
        if (m.x >= title_bar_min.x && m.x <= title_bar_max.x &&
            m.y >= title_bar_min.y && m.y <= title_bar_max.y) {
            if (io.KeyShift) {
                // shift+titlebar -> toggle (jak wyżej)
                if (selectedBlocks.count(box.id))
                    selectedBlocks.erase(box.id);
                else
                    selectedBlocks.insert(box.id);
            } else {
                // BEZ SHIFT: jeżeli box NIE jest aktualnie zaznaczony -> wybierz tylko ten
                // jeżeli box JEST zaznaczony -> NIE czyścimy zaznaczeń (tak, żeby drag uruchomił grupowe przesunięcie)
                if (!selectedBlocks.count(box.id)) {
                    selectedBlocks.clear();
                    selectedBlocks.insert(box.id);
                }
                // else: jeśli już był zaznaczony, zostawiamy selection tak jak jest
            }
        }
    }

    box.drawContent();

    // czy to okno jest przeciągane?
    bool this_window_dragged = false;
    // sprawdź czy mysz jest w titlebarze i trzymamy LPM (potencjalne rozpoczęcie drag)
    if (ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 tb_min = title_bar_min;
        ImVec2 tb_max = title_bar_max;

        if (mouse_pos.x >= tb_min.x && mouse_pos.x <= tb_max.x && mouse_pos.y >= tb_min.y && mouse_pos.y <= tb_max.y)
            this_window_dragged = true;
    }

    // rozpoczęcie przeciągania: jeśli klikamy w titlebar i nie jesteśmy już w stanie dragging -> ustaw stany
    if (this_window_dragged && !isDraggingWindow) {
        isDraggingWindow = true;
        draggedWindowId = box.id;

        // jeżeli ten box jest częścią zaznaczenia i zaznaczono więcej niż 1 -> rozpocznij grupowe przeciąganie
        if (selectedBlocks.count(box.id) && selectedBlocks.size() > 1) {
            isGroupDragging = true;
            groupDragStartMousePos = ImGui::GetMousePos();
            groupInitialPositions.clear();

            // zapisz początkowe pozycje wszystkich zaznaczonych
            for (int id : selectedBlocks) {
                auto it = std::find_if(guiCore->model.getBlocks().begin(), guiCore->model.getBlocks().end(), [&](auto& b){ return b->id == id; });
                if (it != guiCore->model.getBlocks().end())
                    groupInitialPositions[id] = (*it)->position;
            }
        } else {
            // pojedyncze przeciąganie
            isGroupDragging = false;
            groupInitialPositions.clear();
        }
    }
    // zakończenie przeciągania
    else if (isDraggingWindow && draggedWindowId == box.id && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        isDraggingWindow = false;
        draggedWindowId = -1;
        isGroupDragging = false;
        groupInitialPositions.clear();
    }

    // punkt połączenia (output) - znaczek "+"
    // TODO: wziąć w pętle w zależności od ilości outputów
    ImVec2 center = ImVec2(current_screen_pos.x + current_screen_size.x - 15, current_screen_pos.y + current_screen_size.y * 0.5f);

    // kursor do InvisibleButton tak, żeby kółko dało się kliknąć
    ImGui::SetCursorScreenPos(ImVec2(center.x - 10, center.y - 10));
    ImGui::InvisibleButton(("##link" + std::to_string(box.id)).c_str(), ImVec2(20, 20));

    bool isHovered = ImGui::IsItemHovered();
    bool isClicked = ImGui::IsItemClicked();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // kolor przycisku w zależności od stanu
    if (box.getNumOutputs() > 0) {
        ImU32 buttonColor = isClicked ? IM_COL32(255, 0, 0, 255) : (isHovered ? IM_COL32(255, 255, 0, 255) : IM_COL32(200, 200, 0, 255));
        draw_list->AddCircleFilled(center, 8.0f, buttonColor);
        draw_list->AddText(ImVec2(center.x - 4, center.y - 7), IM_COL32(0, 0, 0, 255), "+");
    }
    // TODO: aż do tąd pętle

    // obsługa przeciągania połączeń - tylko gdy nie przeciągamy okna
    if (!isDraggingWindow || draggedWindowId != box.id) {
        if (isClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            if (box.getNumOutputs() > 0)
                guiCore->connectionManager.draggingFrom = box.id;
        }

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            if (box.getNumOutputs() > 0)
                guiCore->connectionManager.draggingFrom = box.id;
        }
    }

    // jeśli trwa grupowe przeciąganie - zaktualizuj pozycje wszystkich zaznaczonych boxów
    if (isGroupDragging && isDraggingWindow && draggedWindowId == box.id && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mouseNow = ImGui::GetMousePos();
        ImVec2 deltaMouse = ImVec2(mouseNow.x - groupDragStartMousePos.x, mouseNow.y - groupDragStartMousePos.y);
        ImVec2 worldDelta = ImVec2(deltaMouse.x / guiCore->viewportManager.zoomAmount, deltaMouse.y / guiCore->viewportManager.zoomAmount);

        for (auto& kv : groupInitialPositions) {
            int id = kv.first;
            ImVec2 basePos = kv.second;
            auto it = std::find_if(guiCore->model.getBlocks().begin(), guiCore->model.getBlocks().end(), [&](auto& b){ return b->id == id; });
            if (it != guiCore->model.getBlocks().end())
                (*it)->position = ImVec2(basePos.x + worldDelta.x, basePos.y + worldDelta.y);
        }
    } else if (!isGroupDragging) {
        // normalne aktualizowanie pozycji tylko tego boxu (po rysowaniu, aby zachować synchronizację)
        box.position = ImVec2(
            (current_screen_pos.x - guiCore->viewportManager.viewOffset.x) / guiCore->viewportManager.zoomAmount,
            (current_screen_pos.y - guiCore->viewportManager.viewOffset.y) / guiCore->viewportManager.zoomAmount
        );
    }

    // zawsze aktualizuj size
    box.size = ImVec2(
        current_screen_size.x / guiCore->viewportManager.zoomAmount,
        current_screen_size.y / guiCore->viewportManager.zoomAmount
    );

    ImGui::GetIO().FontGlobalScale = 1.0f;

    // zdejmij styl jeśli był założony
    if (pushedSelectionStyle)
        ImGui::PopStyleColor(2);

    ImGui::End();
}