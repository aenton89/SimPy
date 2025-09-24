//
// Created by tajbe on 18.04.2025.
//

#include "guiClass.h"
#include "structures.h"
#include <functional>
#include <iostream>
#include <thread>
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "math/solvers/solverMethod.h"



// initialization of evertything regarding ImGui
void guiClass::init(GLFWwindow* win, const char* version) {
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


void guiClass::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void guiClass::update() {
    ImGuiIO& io = ImGui::GetIO();

    // logika zaznaczania/odznaczania boxów
    clearSelectedBlocks(io);
    duplicateSelectedBlocks(io);
    deleteSelectedBlocks(io);
    selectAllBlocks(io);

    // skróty klawiszowe
    turnLightModeOn(io);
    turnGridOn(io);

    // wywołaj nowe funkcje
    drawMenu();
    drawStartButton();
    drawMenuBar();
    noLightMode();

    // Reszta kodu pozostaje bez zmian
    for (auto& box : model.getBlocks()) {
        if (box->open)
            drawBox(*box);
    }

    drawConnections();
    zoom();

    // zbieramy ID boxów do usunięcia
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
    drawGrid();
}

// logika czyszczenia zaznaczeń
void guiClass::clearSelectedBlocks(const ImGuiIO &io) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyShift) {
        ImVec2 mousePos = io.MousePos;
        bool clickedOnAnyTitle = false;
        for (auto& b : model.getBlocks()) {
            ImVec2 screen_pos = ImVec2(b->position.x * zoomAmount + viewOffset.x, b->position.y * zoomAmount + viewOffset.y);
            ImVec2 screen_size = ImVec2(b->size.x * zoomAmount, b->size.y * zoomAmount);
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
void guiClass::duplicateSelectedBlocks(const ImGuiIO &io) {
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D, false)) {
        if (!selectedBlocks.empty()) {
            std::set<int> newSelection;

            for (int id : selectedBlocks) {
                auto it = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b){ return b->id == id; });
                if (it != model.getBlocks().end()) {
                    std::shared_ptr<Block> copy = (*it)->clone();
                    copy->id = model.next_id++;
                    copy->position = ImVec2((*it)->position.x + 20, (*it)->position.y + 20);

                    newSelection.insert(copy->id);
                    model.getBlocks().push_back(std::move(copy));
                }
            }

            // przenosimy zaznaczenie na nowo dodane bloczki
            selectedBlocks = newSelection;
        }
    }
}

// usuwanie bloczków przez DEL
void guiClass::deleteSelectedBlocks(const ImGuiIO &io) {
    if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
        if (!selectedBlocks.empty()) {
            auto& blocks = model.getBlocks();
            blocks.erase(std::remove_if(blocks.begin(), blocks.end(),[&](auto& b) {return selectedBlocks.count(b->id) > 0;}), blocks.end());

            // wyczyść zaznaczenie
            selectedBlocks.clear();
        }
    }
}

// zaznaczanie wszystkich blocków przez CTRL+A
void guiClass::selectAllBlocks(const ImGuiIO& io) {
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false)) {
        selectedBlocks.clear();
        for (const auto& block : model.getBlocks()) {
            selectedBlocks.insert(block->id);
        }
    }
}

// włączanie i wyłączanie light mode przez CTRL+L
void guiClass::turnLightModeOn(const ImGuiIO &io) {
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_L, false))
        lightMode = !lightMode;
}

// włączanie i wyłączanie siatki przez CTRL+G
void guiClass::turnGridOn(const ImGuiIO &io) {
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_G, false))
        gridEnabled = !gridEnabled;
}



// funkcja pomocnicza do obliczania pozycji zadockowanego okna
ImVec2 guiClass::calculateDockedPosition(DockPosition position, DockableWindowType windowType) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    switch (position) {
        case DockPosition::Left:
            // jeśli RUN WINDOW i MENU WINDOW jest już zadockowane po lewej, umieść poniżej
            if (windowType == DockableWindowType::Start && menuWindow.isDocked && (menuWindow.position == DockPosition::Left || menuWindow.position == DockPosition::Top))
                return ImVec2(0, DEFAULT_DOCKED_MENU_SIZE.y + 1 + 20);
            return ImVec2(0, 20);

        case DockPosition::Right:
            if (windowType == DockableWindowType::Start && menuWindow.isDocked && menuWindow.position == DockPosition::Right)
                return ImVec2(displaySize.x - DEFAULT_DOCKED_START_SIZE.x, DEFAULT_DOCKED_MENU_SIZE.y + 1);
            return ImVec2(displaySize.x - DEFAULT_DOCKED_MENU_SIZE.x, 0);

        case DockPosition::Top:
            // rozsuń okna w poziomie
            if (windowType == DockableWindowType::Start && menuWindow.isDocked && (menuWindow.position == DockPosition::Left || menuWindow.position == DockPosition::Top))
                return ImVec2(DEFAULT_DOCKED_MENU_SIZE.x + 1, 20);
            return ImVec2(0, 20);

        case DockPosition::Bottom:
            if (windowType == DockableWindowType::Start && menuWindow.isDocked && menuWindow.position == DockPosition::Bottom)
                return ImVec2(DEFAULT_DOCKED_MENU_SIZE.x + 1, displaySize.y - DEFAULT_DOCKED_START_SIZE.y);
            return ImVec2(0, displaySize.y - DEFAULT_DOCKED_MENU_SIZE.y);

        default:
            return ImVec2(100, 100);
    }
}

ImVec2 guiClass::calculateDockedSize(DockPosition position, DockableWindowType windowType) {
    if (windowType == DockableWindowType::Start)
        return DEFAULT_DOCKED_START_SIZE;
    return DEFAULT_DOCKED_MENU_SIZE;
}

// funkcja do sprawdzania czy okno powinno się zadockować
guiClass::DockPosition guiClass::checkDockPosition(ImVec2 windowPos, ImVec2 windowSize) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    std::cout << "Checking dock position - windowPos: " << windowPos.x << "," << windowPos.y << " size: " << windowSize.x << "," << windowSize.y << " displaySize: " << displaySize.x << "," << displaySize.y << std::endl;

    // lewa krawędź
    if (windowPos.x < dockSnapDistance) {
        std::cout << "Should dock LEFT" << std::endl;
        return DockPosition::Left;
    }
    // prawą krawędź
    if (windowPos.x + windowSize.x > displaySize.x - dockSnapDistance) {
        std::cout << "Should dock RIGHT" << std::endl;
        return DockPosition::Right;
    }
    // górna krawędź
    if (windowPos.y < dockSnapDistance) {
        std::cout << "Should dock TOP" << std::endl;
        return DockPosition::Top;
    }
    // dolną krawędź
    if (windowPos.y + windowSize.y > displaySize.y - dockSnapDistance) {
        std::cout << "Should dock BOTTOM" << std::endl;
        return DockPosition::Bottom;
    }

    return DockPosition::None;
}

void guiClass::drawGrid() {
    if (!gridEnabled)
        return;

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 canvas_pos = ImVec2(0, 20);
    ImVec2 canvas_size = ImVec2(io.DisplaySize.x, io.DisplaySize.y - 20);

    // calculate world space bounds visible on screen
    ImVec2 world_min = ImVec2(
        (canvas_pos.x - viewOffset.x) / zoomAmount,
        (canvas_pos.y - viewOffset.y) / zoomAmount
    );
    ImVec2 world_max = ImVec2(
        (canvas_pos.x + canvas_size.x - viewOffset.x) / zoomAmount,
        (canvas_pos.y + canvas_size.y - viewOffset.y) / zoomAmount
    );

    // use fixed grid spacing in world coordinates (like Blender [yupii blender mentioned])
    float world_spacing = gridSpacing;

    // calculate grid start positions
    float start_x = floor(world_min.x / world_spacing) * world_spacing;
    float start_y = floor(world_min.y / world_spacing) * world_spacing;

    // make grid lines fade out when too close or far
    float screen_spacing = world_spacing * zoomAmount;
    float alpha_factor = 1.0f;

    // fade out when too dense
    if (screen_spacing < 10.0f) {
        alpha_factor = screen_spacing / 10.0f;
    }
    // fade out when too far
    else if (screen_spacing > 200.0f) {
        alpha_factor = 200.0f / screen_spacing;
    }

    // apply alpha to grid color
    ImU32 faded_color = (gridColor & 0x00FFFFFF) | ((int)(((gridColor >> 24) & 0xFF) * alpha_factor) << 24);

    // skip drawing if too faded
    if (alpha_factor < 0.1f)
        return;

    // vertical lines
    for (float x = start_x; x <= world_max.x + world_spacing; x += world_spacing) {
        float screen_x = x * zoomAmount + viewOffset.x;

        if (screen_x < canvas_pos.x - 1 || screen_x > canvas_pos.x + canvas_size.x + 1)
            continue;

        draw_list->AddLine(
            ImVec2(screen_x, canvas_pos.y),
            ImVec2(screen_x, canvas_pos.y + canvas_size.y),
            faded_color,
            gridThickness
        );
    }

    // horizontal lines
    for (float y = start_y; y <= world_max.y + world_spacing; y += world_spacing) {
        float screen_y = y * zoomAmount + viewOffset.y;

        if (screen_y < canvas_pos.y - 1 || screen_y > canvas_pos.y + canvas_size.y + 1)
            continue;

        draw_list->AddLine(
            ImVec2(canvas_pos.x, screen_y),
            ImVec2(canvas_pos.x + canvas_size.x, screen_y),
            faded_color,
            gridThickness
        );
    }
}

void guiClass::noLightMode() {
    if (lightMode) {
        if (ImGui::Begin("the WHAT mode!?", nullptr, ImGuiWindowFlags_NoResize)) {
            if (ImGui::IsWindowAppearing())
                ImGui::SetWindowSize(ImVec2(200, 100));

            float windowWidth = ImGui::GetWindowSize().x;

            const char* title = "bitch PLEASE";
            ImVec2 textSize = ImGui::CalcTextSize(title);
            ImGui::SetCursorPosX((windowWidth - textSize.x) * 0.5f);
            ImGui::Text("%s", title);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            const char* label = "Light Mode";
            ImVec2 checkSize = ImGui::CalcTextSize(label);
            ImGui::SetCursorPosX((windowWidth - checkSize.x - ImGui::GetFrameHeight()) * 0.5f);
            ImGui::Checkbox(label, &lightMode);

            ImGui::End();
        }
    }
}




void guiClass::zoom() {
    ImGuiIO& io = ImGui::GetIO();

    // obsługa zoom'u względem pozycji myszki
    if (io.MouseWheel != 0.0f && !canvasDragging && !isDraggingWindow) {
        float zoom_speed = 0.1f;
        float old_zoom = zoomAmount;

        zoomAmount += io.MouseWheel * zoom_speed;
        zoomAmount = std::clamp(zoomAmount, 0.1f, 10.0f);

        if (zoomAmount != old_zoom) {
            ImVec2 mouse_pos = io.MousePos;
            ImVec2 mouse_world_before = ImVec2(
                (mouse_pos.x - viewOffset.x) / old_zoom,
                (mouse_pos.y - viewOffset.y) / old_zoom
            );
            ImVec2 mouse_world_after = ImVec2(
                (mouse_pos.x - viewOffset.x) / zoomAmount,
                (mouse_pos.y - viewOffset.y) / zoomAmount
            );
            viewOffset.x += (mouse_world_after.x - mouse_world_before.x) * zoomAmount;
            viewOffset.y += (mouse_world_after.y - mouse_world_before.y) * zoomAmount;
        }
    }

    // rozpoczęcie przeciągania canvas'u - pod (TYLKO) środkowym przyciskiem
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && !ImGui::IsAnyItemActive() && !isDraggingWindow && !canvasDragging) {
        canvasDragging = true;
        dragStartPos = io.MousePos;
        dragStartOffset = viewOffset;
        std::cout << "Started canvas dragging at: " << dragStartPos.x << ", " << dragStartPos.y << std::endl;
    }

    // kontynuacja przeciągania canvas'u
    if (canvasDragging) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
            ImVec2 delta = ImVec2(io.MousePos.x - dragStartPos.x, io.MousePos.y - dragStartPos.y);
            viewOffset = ImVec2(dragStartOffset.x + delta.x, dragStartOffset.y + delta.y);
            std::cout << "Canvas dragging - delta: " << delta.x << ", " << delta.y << " offset: " << viewOffset.x << ", " << viewOffset.y << std::endl;
        } else {
            // środkowy przycisk został puszczony
            canvasDragging = false;
            std::cout << "Stopped canvas dragging" << std::endl;
        }
    }

    // jeśli przeciągamy okno, zatrzymaj przeciąganie canvas'u
    if (isDraggingWindow && canvasDragging) {
        canvasDragging = false;
        std::cout << "Canvas dragging stopped due to window dragging" << std::endl;
    }
}

void guiClass::applyCanvasTransform() {
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    // zastosuj transformację do wszystkich operacji rysowania - będzie działać dla linii połączeń
    draw_list->_VtxCurrentIdx = 0;
}


void guiClass::drawBox(Block& box) {
    std::string title = "Box #" + std::to_string(box.id);

    // oblicz transformowaną pozycję
    ImVec2 world_pos = box.position;
    ImVec2 screen_pos = ImVec2(
        world_pos.x * zoomAmount + viewOffset.x,
        world_pos.y * zoomAmount + viewOffset.y
    );
    ImVec2 screen_size = ImVec2(
        box.size.x * zoomAmount,
        box.size.y * zoomAmount
    );

    // MOLTO IMPORTANTE: NIE ustawiaj pozycji jeśli okno jest przeciągane przez użytkownika
    if (!ImGui::IsWindowAppearing() && !isDraggingWindow)
        ImGui::SetNextWindowPos(screen_pos);
    // inne okna nadal powinny być pozycjonowane
    else if (isDraggingWindow && draggedWindowId != box.id)
        ImGui::SetNextWindowPos(screen_pos);

    ImGui::SetNextWindowSize(screen_size);

    // skaluj font
    ImGui::GetIO().FontGlobalScale = zoomAmount;

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
                auto it = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b){ return b->id == id; });
                if (it != model.getBlocks().end())
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
                dragging_from = box.id;
        }

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            if (box.getNumOutputs() > 0)
                dragging_from = box.id;
        }
    }

    // jeśli trwa grupowe przeciąganie - zaktualizuj pozycje wszystkich zaznaczonych boxów
    if (isGroupDragging && isDraggingWindow && draggedWindowId == box.id && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mouseNow = ImGui::GetMousePos();
        ImVec2 deltaMouse = ImVec2(mouseNow.x - groupDragStartMousePos.x, mouseNow.y - groupDragStartMousePos.y);
        ImVec2 worldDelta = ImVec2(deltaMouse.x / zoomAmount, deltaMouse.y / zoomAmount);

        for (auto& kv : groupInitialPositions) {
            int id = kv.first;
            ImVec2 basePos = kv.second;
            auto it = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b){ return b->id == id; });
            if (it != model.getBlocks().end())
                (*it)->position = ImVec2(basePos.x + worldDelta.x, basePos.y + worldDelta.y);
        }
    } else if (!isGroupDragging) {
        // normalne aktualizowanie pozycji tylko tego boxu (po rysowaniu, aby zachować synchronizację)
        box.position = ImVec2(
            (current_screen_pos.x - viewOffset.x) / zoomAmount,
            (current_screen_pos.y - viewOffset.y) / zoomAmount
        );
    }

    // zawsze aktualizuj size
    box.size = ImVec2(
        current_screen_size.x / zoomAmount,
        current_screen_size.y / zoomAmount
    );

    ImGui::GetIO().FontGlobalScale = 1.0f;

    // zdejmij styl jeśli był założony
    if (pushedSelectionStyle)
        ImGui::PopStyleColor(2);

    ImGui::End();
}


void guiClass::drawConnections() {
    // dodaj tę linię na początku
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();

    for (auto& box : model.getBlocks()) {
        for (auto it = box->connections.begin(); it != box->connections.end();) {
            // Szukamy boxa docelowego (target)
            auto targetIt = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b) { return b->id == *it; });
            if (targetIt != model.getBlocks().end()) {
                // oblicz pozycje w przestrzeni ekranu (po transformacji)
                ImVec2 target_pos = ImVec2(
                    (*targetIt)->position.x * zoomAmount + viewOffset.x,
                    (*targetIt)->position.y * zoomAmount + viewOffset.y
                );
                ImVec2 target_size = ImVec2(
                    (*targetIt)->size.x * zoomAmount,
                    (*targetIt)->size.y * zoomAmount
                );

                ImVec2 source_pos = ImVec2(
                    box->position.x * zoomAmount + viewOffset.x,
                    box->position.y * zoomAmount + viewOffset.y
                );
                ImVec2 source_size = ImVec2(
                    box->size.x * zoomAmount,
                    box->size.y * zoomAmount
                );

                // punkty połączenia
                ImVec2 p1 = ImVec2(target_pos.x + target_size.x, target_pos.y + target_size.y * 0.5f);
                ImVec2 p2 = ImVec2(source_pos.x, source_pos.y + source_size.y * 0.5f);

                // sprawdź hover na linii
                const float detect_radius = 10.0f * zoomAmount;
                bool hovered = false;

                // sample line geometry in 20 steps and check distance to mouse
                for (int step = 0; step <= 20; ++step) {
                    float t = step / 20.0f;
                    ImVec2 pt = BezierCubicCalc(p1, ImVec2(p1.x + 50 * zoomAmount, p1.y), ImVec2(p2.x - 50 * zoomAmount, p2.y), p2, t);
                    if (LengthSqr(pt, mousePos) < detect_radius * detect_radius) {
                        hovered = true;
                        break;
                    }
                }

                // rysowanie z uwzględnieniem skali
                ImU32 color = hovered ? IM_COL32(255, 100, 100, 255) : IM_COL32(255, 255, 0, 255);
                float thickness = (hovered ? 5.0f : 3.0f) * zoomAmount;

                // rysowanie krzywej
                draw_list->AddBezierCubic(
                    p1,
                    ImVec2(p1.x + 50 * zoomAmount, p1.y),
                    ImVec2(p2.x - 50 * zoomAmount, p2.y),
                    p2,
                    color,
                    thickness
                );

                // kliknięcie PPM = rozłączenie
                if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    it = box->connections.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    // dynamiczna linia podczas przeciągania
    if (dragging_from) {
        auto it = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b) { return b->id == *dragging_from; });
        if (it != model.getBlocks().end()) {
            ImVec2 source_pos = ImVec2(
                (*it)->position.x * zoomAmount + viewOffset.x,
                (*it)->position.y * zoomAmount + viewOffset.y
            );
            ImVec2 source_size = ImVec2(
                (*it)->size.x * zoomAmount,
                (*it)->size.y * zoomAmount
            );

            ImVec2 p1 = ImVec2(source_pos.x + source_size.x, source_pos.y + source_size.y * 0.5f);
            // P2 to koniec (mysz)
            ImVec2 p2 = ImGui::GetMousePos();

            draw_list->AddBezierCubic(
                p1,
                ImVec2(p1.x + 50 * zoomAmount, p1.y),
                ImVec2(p2.x - 50 * zoomAmount, p2.y),
                p2,
                IM_COL32(255, 255, 0, 100),
                2.0f * zoomAmount
            );
        }
    }

    // zakończenie przeciągania z uwzględnieniem transformacji
    if (dragging_from && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        bool connected = false;

        for (auto& box : model.getBlocks()) {
            // przekształć pozycję boxa do przestrzeni ekranu
            ImVec2 transformed_pos = ImVec2(
                box->position.x * zoomAmount + viewOffset.x,
                box->position.y * zoomAmount + viewOffset.y
            );
            ImVec2 transformed_size = ImVec2(
                box->size.x * zoomAmount,
                box->size.y * zoomAmount
            );

            ImVec2 boxMin = transformed_pos;
            ImVec2 boxMax = ImVec2(boxMin.x + transformed_size.x, boxMin.y + transformed_size.y);

            if (mousePos.x >= boxMin.x && mousePos.x <= boxMax.x && mousePos.y >= boxMin.y && mousePos.y <= boxMax.y) {
                // zmieniamy kierunek połączenia
                if (box->id != *dragging_from) {
                    if (box->getNumInputs() > 0)
                        box->connections.push_back(*dragging_from);
                }
                connected = true;
                break;
                }
        }

        dragging_from = std::nullopt;
    }
}


void guiClass::drawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) { /* akcja */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* akcja */ }
            if (ImGui::MenuItem("Exit", "Alt+F4")) { /* akcja */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* akcja */ }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) { /* akcja */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            ImGui::MenuItem("Light mode", "Ctrl+L", &lightMode);
            ImGui::MenuItem("Show grid", "Ctrl+G", &gridEnabled);

            if (ImGui::BeginMenu("Grid Settings")) {
                ImGui::SliderFloat("Grid Spacing", &gridSpacing, 10.0f, 200.0f);
                ImGui::SliderFloat("Line Thickness", &gridThickness, 0.5f, 5.0f);

                // color picker for grid
                ImVec4 grid_color = ImGui::ColorConvertU32ToFloat4(gridColor);
                if (ImGui::ColorEdit4("Grid Color", (float*)&grid_color, ImGuiColorEditFlags_AlphaPreview)) {
                    gridColor = ImGui::ColorConvertFloat4ToU32(grid_color);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void guiClass::drawMenu() {
    ImVec2 menuPos, menuSize;
    ImGuiWindowFlags menuFlags = ImGuiWindowFlags_AlwaysAutoResize;

    if (menuWindow.isDocked) {
        menuSize = calculateDockedSize(menuWindow.position, DockableWindowType::Menu);
        menuPos = calculateDockedPosition(menuWindow.position, DockableWindowType::Menu);
        menuFlags |= ImGuiWindowFlags_NoMove;
    } else {
        menuPos = menuWindow.undockedPos;
    }

    ImGui::SetNextWindowPos(menuPos, menuWindow.isDocked ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::SetNextWindowSize(menuSize, menuWindow.isDocked ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(160, 150), ImVec2(FLT_MAX, FLT_MAX));

    if (ImGui::Begin("Menu", nullptr, menuFlags)) {
        // przyciski dodawania bloków

        // modul math
        if (ImGui::CollapsingHeader("Math")) {
            if (ImGui::Button("Add Sum Box"))
                model.addBlock<SumBlock>();
            if (ImGui::Button("Add Multiply Box"))
                model.addBlock<MultiplyBlock>();
            if (ImGui::Button("Add Integrator Box"))
                model.addBlock<IntegratorBlock>();
            if (ImGui::Button("Add Diff Box"))
                model.addBlock<DifferentiatorBlock>();
            if (ImGui::Button("Add Trigonometric Funcion Box"))
                model.addBlock<TrigonometricFunctionBlock>();
            if (ImGui::Button("Add Sqrt Box"))
                model.addBlock<sqrtBlock>();
            if (ImGui::Button("Add Squered Box"))
                model.addBlock<squaredBlock>();
        }

        // modul contorl
        if (ImGui::CollapsingHeader("Control Continous")) {
            if (ImGui::Button("Add Tf box"))
                model.addBlock<TransferFuncionContinous>();
            if (ImGui::Button("Add PID box"))
                model.addBlock<PID_regulator>();
            if (ImGui::Button("Add Gain Box"))
                model.addBlock<GainBlock>();
            if (ImGui::Button("Add Saturation Box"))
                model.addBlock<SaturationBlock>();
            if (ImGui::Button("Add DeadZone Box"))
                model.addBlock<DeadZoneBlock>();
        }

        // modul inputy
        if (ImGui::CollapsingHeader("Input")) {
            if (ImGui::Button("Add Step Box"))
                model.addBlock<StepBlock>();
            if (ImGui::Button("Add SinusInput Box"))
                model.addBlock<SinusInputBlock>();
            if (ImGui::Button("Add PWM Input"))
                model.addBlock<PWMInputBlock>();
            if (ImGui::Button("Add WhiteNoise Box"))
                model.addBlock<WhiteNoiseInputBlock>();
            if (ImGui::Button("Add Input form File"))
                model.addBlock<SignalFromFileBlock>();
        }

        // modul spketum czestoliwosciowego
        if (ImGui::CollapsingHeader("DSP")) {
            if (ImGui::Button("Add STFT Box (work in progres)"))
                model.addBlock<STFT_block>();
            if (ImGui::Button("Add filter Box"))
                model.addBlock<filterImplementationBlock>();
            if (ImGui::Button("Add MovAvg Box"))
                model.addBlock<meanFilter1DBlock>();
            if (ImGui::Button("Add MedianFilter Box"))
                model.addBlock<medianFilter1DBlock>();
        }

        // modul print/ploty
        if (ImGui::CollapsingHeader("Print")) {
            if (ImGui::Button("Add Print Box"))
                model.addBlock<PrintBlock>();
            if (ImGui::Button("Add Plot Box"))
                model.addBlock<PlotBlock>();
            if (ImGui::Button("Add Plot XY Box"))
                model.addBlock<PlotXYBlock>();
            if (ImGui::Button("Add Spectogram Box (work in progres)"))
                model.addBlock<PlotHeatmapBlock>();
        }

        // moduł code
        if (ImGui::CollapsingHeader("Code Box")) {
            if (ImGui::Button("Add Python Box"))
                model.addBlock<pythonBlock>();
            if (ImGui::Button("Add C++ Box"))
                model.addBlock<cppBlock>();
        }

        // bloki logiczne (sprawdzenie czy aktualna struktra sie do tego nadaje)
        if (ImGui::CollapsingHeader("Logic")) {
            if (ImGui::Button("Add OR Box"))
                model.addBlock<logicORBlock>();
            if (ImGui::Button("Add AND Box"))
                model.addBlock<logicANDBlock>();
            if (ImGui::Button("Add NOT Box"))
                model.addBlock<logicNOTBlock>();
            if (ImGui::Button("Add NOR Box"))
                model.addBlock<logicNORBlock>();
        }

        // wysylanie danych
        if (ImGui::CollapsingHeader("Sender")) {
            if (ImGui::Button("Add Sender Box"))
                model.addBlock<DataSenderBlock>();
        }

        ImGui::Separator();

        // przycisk do undocking
        if (menuWindow.isDocked) {
            if (ImGui::Button("Undock Menu")) {
                menuWindow.isDocked = false;
                menuWindow.position = DockPosition::None;

                // przesuń okno z dala od krawędzi i zresetuj rozmiar
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 displaySize = io.DisplaySize;
                menuWindow.undockedPos = ImVec2(displaySize.x * 0.3f, displaySize.y * 0.3f);
            }
        }

        // sprawdź docking tylko jeśli okno nie jest zadockowane
        if (!menuWindow.isDocked) {
            ImVec2 currentPos = ImGui::GetWindowPos();
            ImVec2 currentSize = ImGui::GetWindowSize();

            // zapisz pozycję jako undocked
            menuWindow.undockedPos = currentPos;

            // sprawdź czy powinno się zadockować
            static bool wasDragging = false;
            bool isDragging = ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowFocused();

            if (wasDragging && !isDragging) {
                DockPosition newDockPos = checkDockPosition(currentPos, currentSize);
                if (newDockPos != DockPosition::None) {
                    menuWindow.isDocked = true;
                    menuWindow.position = newDockPos;
                }
            }

            wasDragging = isDragging;
        }
    }
    ImGui::End();
}

void guiClass::drawStartButton() {
    ImVec2 startPos, startSize;
    ImGuiWindowFlags startFlags = ImGuiWindowFlags_AlwaysAutoResize;

    if (startWindow.isDocked) {
        startSize = calculateDockedSize(startWindow.position, DockableWindowType::Start);
        startPos = calculateDockedPosition(startWindow.position, DockableWindowType::Start);
        startFlags |= ImGuiWindowFlags_NoMove;
    } else {
        startPos = startWindow.undockedPos;
    }

    ImGui::SetNextWindowPos(startPos, startWindow.isDocked ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::SetNextWindowSize(startSize, startWindow.isDocked ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(160, 150), ImVec2(FLT_MAX, FLT_MAX));


    if (ImGui::Begin("Start / Stop", nullptr, startFlags)) {
        // szerokość w pikselach
        ImGui::PushItemWidth(70.0);
        ImGui::InputDouble("Sampling Time", &Model::timeStep);
        ImGui::InputDouble("Simulation Time", &Model::simTime);

        // combo odpowiedialne za wybor solver i precyzje obliczen (musisz Antek zrobic templeta do tego bo ja nie umim XD)
        static int current_solver = 2;
        const static char* solvers[] = {"RK1", "RK2", "RK4", "CN", "GL", "Gear"};

        static int current_precision = 0;
        const static char* precisions[] = {"int", "float", "double"};

        if (ImGui::BeginCombo("Solver Type", solvers[current_solver], false)) {
            for (int n = 0; n < IM_ARRAYSIZE(solvers); n++) {
                bool is_selected = (current_precision == n);
                if (ImGui::Selectable(solvers[n], is_selected)) {
                    current_solver = n;
                    this->solverName = solvers[n];
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Precision Type", precisions[current_precision], false)) {
            for (int i = 0; i < IM_ARRAYSIZE(precisions); i++) {
                bool is_selected = (current_precision == i);
                if (ImGui::Selectable(precisions[i], is_selected)) {
                    current_precision = i;
                    this->solverPrecison = precisions[i];
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // mapa do wyboru Solvera
        std::unordered_map<std::string, std::function<std::shared_ptr<ISolverMethod>()>> solverMap = {
            {"RK1", [](){ return std::make_shared<RK1Method>(); }},
            {"RK2", [](){ return std::make_shared<RK2Method>(); }},
            {"RK4", [](){ return std::make_shared<RK4Method>(); }},
            {"CN",  [](){ return std::make_shared<RK1Method>(); }},
            {"GL",  [](){ return std::make_shared<RK1Method>(); }},
            {"Gear",[](){ return std::make_shared<RK1Method>(); }}
        };

        if (simulationRunning) {
            ImGui::BeginDisabled();
            ImGui::Button("Running...");
            ImGui::EndDisabled();
        } else {
            if (ImGui::Button("Run Simulation")) {
                simulationRunning = true;
                auto method = solverMap[this->solverName]();
                SolverManager::initSolver(Model::timeStep, method);
                // uruchom w osobnym wątku i nie czekaj na niego:
                std::thread([this]() {
                    // cleanup w bloczkach jeśli jest potrzeb
                    model.cleanupBefore();
                    model.makeConnections();
                    // TODO: tu na ogół nie ma być na stałe pętli do 1000
                    // patryk chyba chciał to jakoś ustawiać, idk w sumie nie pamiętam
                    for (int i = 0; i < (Model::simTime/ Model::timeStep) + 1; i++) {
                        model.simulate();
                    }
                    model.cleanupAfter();
                    model.cleanSolver();
                    //model.getBlocks().clear();
                    simulationRunning = false;
                }).detach();
            }
        }

        ImGui::Separator();

        if (startWindow.isDocked) {
            if (ImGui::Button("Undock Window")) {
                startWindow.isDocked = false;
                startWindow.position = DockPosition::None;

                // przesuń okno z dala od krawędzi i zresetuj rozmiar
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 displaySize = io.DisplaySize;
                startWindow.undockedPos = ImVec2(displaySize.x * 0.5f, displaySize.y * 0.4f);
                // domyślny rozmiar
            }
        }

        if (!startWindow.isDocked) {
            ImVec2 currentPos = ImGui::GetWindowPos();
            ImVec2 currentSize = ImGui::GetWindowSize();

            startWindow.undockedPos = currentPos;

            // taka sama logika jak dla menu window
            static bool wasDraggingStart = false;
            bool isDragging = ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowFocused();

            if (wasDraggingStart && !isDragging) {
                DockPosition newDockPos = checkDockPosition(currentPos, currentSize);
                if (newDockPos != DockPosition::None) {
                    startWindow.isDocked = true;
                    startWindow.position = newDockPos;
                }
            }

            wasDraggingStart = isDragging;
        }
    }
    ImGui::End();
}


void guiClass::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void guiClass::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();
}
