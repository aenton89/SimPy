//
// Created by tajbe on 10.11.2025.
//
#include "BlocksManager.h"
#include "../../GUICore.h"



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
                auto it = std::ranges::find_if(guiCore->model.getBlocks(), [&](auto& b){ return b->id == id; });
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
            // usuwa pozostałe połączenia
            guiCore->connectionManager.removeConnectionsForBlocks(selectedBlocks);

            // usuwa same bloki
            auto& blocks = guiCore->model.getBlocks();
            std::erase_if(blocks,[&](auto& b) {return selectedBlocks.contains(b->id);});

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

void BlocksManager::drawBlock(const std::shared_ptr<Block> &box) {
    std::string title = "Box #" + std::to_string(box->id);

    // oblicz transformowaną pozycję
    ImVec2 world_pos = box->position;
    ImVec2 screen_pos = ImVec2(
        world_pos.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
        world_pos.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
    );
    ImVec2 screen_size = ImVec2(
        box->size.x * guiCore->viewportManager.zoomAmount,
        box->size.y * guiCore->viewportManager.zoomAmount
    );

    // MOLTO IMPORTANTE: NIE ustawiaj pozycji jeśli okno jest przeciągane przez użytkownika
    if ((!ImGui::IsWindowAppearing() && !isDraggingWindow) || (isDraggingWindow && draggedWindowId != box->id))
        ImGui::SetNextWindowPos(screen_pos);

    // inne okna nadal powinny być pozycjonowane
    ImGui::SetNextWindowSize(screen_size);

    // skaluj font
    ImGui::GetIO().FontGlobalScale = guiCore->viewportManager.zoomAmount;

    // jeśli box jest zaznaczony - ustaw niebieskie titlebary
    bool pushedSelectionStyle = false;
    if (selectedBlocks.contains(box->id)) {
        // onegdaj niebieski, teraz takied śmiszne cuś
        ImGui::PushStyleColor(ImGuiCol_TitleBg, UIStyles::SELECTION_COLOR);
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, UIStyles::SELECTION_COLOR);
        pushedSelectionStyle = true;
    }

    // flagi okna - pozwól na normalne przeciąganie
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

    if (!ImGui::Begin(title.c_str(), &box->open, flags)) {
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
            if (selectedBlocks.contains(box->id))
                selectedBlocks.erase(box->id);
            else
                selectedBlocks.insert(box->id);
        }
    }

    // klik w titlebar (LPM):
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 m = io.MousePos;
        if (m.x >= title_bar_min.x && m.x <= title_bar_max.x &&
            m.y >= title_bar_min.y && m.y <= title_bar_max.y) {
            if (io.KeyShift) {
                // shift+titlebar -> toggle (jak wyżej)
                if (selectedBlocks.contains(box->id))
                    selectedBlocks.erase(box->id);
                else
                    selectedBlocks.insert(box->id);
            } else {
                // BEZ SHIFT: jeżeli box NIE jest aktualnie zaznaczony -> wybierz tylko ten
                // jeżeli box JEST zaznaczony -> NIE czyścimy zaznaczeń (tak, żeby drag uruchomił grupowe przesunięcie)
                if (!selectedBlocks.contains(box->id)) {
                    selectedBlocks.clear();
                    selectedBlocks.insert(box->id);
                }
                // else: jeśli już był zaznaczony, zostawiamy selection tak jak jest
            }
        }
    }

    box->drawContent();

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
        draggedWindowId = box->id;

        // jeżeli ten box jest częścią zaznaczenia i zaznaczono więcej niż 1 -> rozpocznij grupowe przeciąganie
        if (selectedBlocks.contains(box->id) && selectedBlocks.size() > 1) {
            isGroupDragging = true;
            groupDragStartMousePos = ImGui::GetMousePos();
            groupInitialPositions.clear();

            // zapisz początkowe pozycje wszystkich zaznaczonych
            for (int id : selectedBlocks) {
                auto it = std::ranges::find_if(guiCore->model.getBlocks(), [&](auto& b){ return b->id == id; });
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
    else if (isDraggingWindow && draggedWindowId == box->id && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
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
    ImGui::InvisibleButton(("##link" + std::to_string(box->id)).c_str(), ImVec2(20, 20));

    bool isHovered = ImGui::IsItemHovered();
    bool isClicked = ImGui::IsItemClicked();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // kolor przycisku w zależności od stanu
    if (box->getNumOutputs() > 0) {
        ImU32 buttonColor = isClicked ? UIStyles::BUTTON_ACTIVE: (isHovered ? UIStyles::BUTTON_HOVER : UIStyles::BUTTON_NORMAL);
        draw_list->AddCircleFilled(center, 8.0f, buttonColor);
        draw_list->AddText(ImVec2(center.x - 4, center.y - 7), UIStyles::BUTTON_PLUS_COLOR, "+");
    }
    // TODO: aż do tąd pętle

    // obsługa przeciągania połączeń - tylko gdy nie przeciągamy okna
    if (!isDraggingWindow || draggedWindowId != box->id) {
        // TODO: dodać w GUI opcje wyboru source portu połączenia
        if (isClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            if (box->getNumOutputs() > 0) {
                // takie coś też było po drodze - ale pozwalało na pare output'ów z bloczka, który ma jedne dane wyjściowe
                // dodatkowo sprawdzenie warunku:
                // && guiCore->model.getOutputConnectionsFor(box).size() < box->getNumOutputs()
                // i to wewenątrz if'a:
                // guiCore->connectionManager.startConnectionDraft(box->id, guiCore->model.getOutputConnectionsFor(box).size());
                int outputPort = 0;
                guiCore->connectionManager.startConnectionDraft(box->id, outputPort);
            }
        }
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            if (box->getNumOutputs() > 0) {
                int outputPort = 0;
                guiCore->connectionManager.startConnectionDraft(box->id, outputPort);
            }
        }
    }

    // jeśli trwa grupowe przeciąganie - zaktualizuj pozycje wszystkich zaznaczonych boxów
    if (isGroupDragging && isDraggingWindow && draggedWindowId == box->id && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mouseNow = ImGui::GetMousePos();
        ImVec2 deltaMouse = ImVec2(mouseNow.x - groupDragStartMousePos.x, mouseNow.y - groupDragStartMousePos.y);
        ImVec2 worldDelta = ImVec2(deltaMouse.x / guiCore->viewportManager.zoomAmount, deltaMouse.y / guiCore->viewportManager.zoomAmount);

        for (auto& kv : groupInitialPositions) {
            int id = kv.first;
            ImVec2 basePos = kv.second;
            auto it = std::ranges::find_if(guiCore->model.getBlocks(), [&](auto& b){ return b->id == id; });
            if (it != guiCore->model.getBlocks().end())
                (*it)->position = ImVec2(basePos.x + worldDelta.x, basePos.y + worldDelta.y);
        }
    } else if (!isGroupDragging) {
        // normalne aktualizowanie pozycji tylko tego boxu (po rysowaniu, aby zachować synchronizację)
        box->position = ImVec2(
            (current_screen_pos.x - guiCore->viewportManager.viewOffset.x) / guiCore->viewportManager.zoomAmount,
            (current_screen_pos.y - guiCore->viewportManager.viewOffset.y) / guiCore->viewportManager.zoomAmount
        );
    }

    // zawsze aktualizuj size
    box->size = ImVec2(
        current_screen_size.x / guiCore->viewportManager.zoomAmount,
        current_screen_size.y / guiCore->viewportManager.zoomAmount
    );

    ImGui::GetIO().FontGlobalScale = 1.0f;

    // zdejmij styl jeśli był założony
    if (pushedSelectionStyle)
        ImGui::PopStyleColor(2);

    ImGui::End();
}

// box select - rysowanie prostokąta, który zaznacza bloczki i wykrycie, które bloczki są w nim
void BlocksManager::updateBoxSelect(const ImGuiIO& io) {
    // nie ruszamy box selecta gdy przeciągamy okno bloczka
    if (isDraggingWindow)
        return;

    ImVec2 mousePos = io.MousePos;

    // START - LPM wciśnięty na pustym miejscu (nie na bloczku)
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !guiCore->connectionManager.isDraggingNode()) {
        // sprawdź czy klik był poza każdym bloczkiem
        bool clickedOnBlock = false;

        for (auto& b : guiCore->model.getBlocks()) {
            ImVec2 screen_pos = ImVec2(
                b->position.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
                b->position.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y);
            ImVec2 screen_size = ImVec2(
                b->size.x * guiCore->viewportManager.zoomAmount,
                b->size.y * guiCore->viewportManager.zoomAmount);

            if (mousePos.x >= screen_pos.x && mousePos.x <= screen_pos.x + screen_size.x && mousePos.y >= screen_pos.y && mousePos.y <= screen_pos.y + screen_size.y) {
                clickedOnBlock = true;
                break;
            }
        }
        if (!clickedOnBlock) {
            isBoxSelecting = true;
            boxSelectStart = mousePos;
            boxSelectEnd = mousePos;
        }
    }

    // AKTUALIZACJA: podczas trzymania LPM - przesuwa koniec prostokąta
    if (isBoxSelecting) {
        // jeśli w trakcie box selecta zaczynamy przeciągać połączenie lub węzeł, anuluj box selecta
        if (guiCore->connectionManager.isDraggingNode() || guiCore->connectionManager.isDraftingConnection()) {
            isBoxSelecting = false;
            return;
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            boxSelectEnd = mousePos;

            // rysuj prostokąt zaznaczenia na foreground draw liście
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            ImVec2 rMin = ImVec2(std::min(boxSelectStart.x, boxSelectEnd.x), std::min(boxSelectStart.y, boxSelectEnd.y));
            ImVec2 rMax = ImVec2(std::max(boxSelectStart.x, boxSelectEnd.x), std::max(boxSelectStart.y, boxSelectEnd.y));
            fg->AddRectFilled(rMin, rMax, IM_COL32(100, 160, 255, 40));
            fg->AddRect(rMin, rMax, IM_COL32(100, 160, 255, 200), 0.0f, 0, 1.5f);
        }
        // KONIEC: po puszczeniu LPM - zaznacza bloczki w prostokącie
        else {
            ImVec2 rMin = ImVec2(std::min(boxSelectStart.x, boxSelectEnd.x), std::min(boxSelectStart.y, boxSelectEnd.y));
            ImVec2 rMax = ImVec2(std::max(boxSelectStart.x, boxSelectEnd.x), std::max(boxSelectStart.y, boxSelectEnd.y));

            // zaznaczaj tylko jeśli prostokąt jest większy niż 4 piksele - unikanie przypadkowego kliknięcia
            if (rMax.x - rMin.x > 5.0f || rMax.y - rMin.y > 5.0f) {
                // bez SHIFT czyścimy poprzednie zaznaczenie, z SHIFT dodajemy do niego
                if (!io.KeyShift)
                    selectedBlocks.clear();

                for (auto& b : guiCore->model.getBlocks()) {
                    ImVec2 screen_pos = ImVec2(
                        b->position.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
                        b->position.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y);
                    ImVec2 screen_size = ImVec2(
                        b->size.x * guiCore->viewportManager.zoomAmount,
                        b->size.y * guiCore->viewportManager.zoomAmount);

                    // bloczek zaznaczony jeśli jego prostokąt przecina obszar box selecta
                    ImVec2 bMax = ImVec2(screen_pos.x + screen_size.x, screen_pos.y + screen_size.y);
                    if (screen_pos.x < rMax.x && bMax.x > rMin.x && screen_pos.y < rMax.y && bMax.y > rMin.y)
                        selectedBlocks.insert(b->id);
                }
            }

            isBoxSelecting = false;
        }
    }
}
