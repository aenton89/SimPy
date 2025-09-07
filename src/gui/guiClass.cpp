//
// Created by tajbe on 18.04.2025.
//


#include "guiClass.h"
#include <iostream>
#include <thread>
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



// initialization of evertything regarding ImGui
void guiClass::init(GLFWwindow* win, const char* version) {
    window = win;
    glsl_version = version;

    // jakaś defaultowa ikonka, potem zmienie
    GLFWimage images[1];
    images[0].pixels = stbi_load("../../icon_v3.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
    if (!images[0].pixels) {
        std::cerr << "ERROR: couldn't load icon.png!" << std::endl;
    }

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
    // Wywołaj nowe funkcje
    drawMenu();
    drawStartButton();
    drawMenuBar();


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
        [](const std::unique_ptr<Block>& box) {
            return !box->open;
        }), model.getBlocks().end());
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

// void guiClass::drawBox(Block& box) {
//     std::string title = "Box #" + std::to_string(box.id);
//     ImGui::SetNextWindowPos(box.position, ImGuiCond_Once);
//     ImGui::SetNextWindowSize(ImVec2(box.size.x, box.size.y), ImGuiCond_Once);
//
//     bool is_hovered = false;
//
//     if (!ImGui::Begin(title.c_str(), &box.open)) {
//         ImGui::End();
//         return;
//     }
//
//     box.drawContent();
//
//     // znak + po lewej stronie box'a
//     // pozycję i rozmiar okna boxa
//     ImVec2 windowPos = ImGui::GetWindowPos();
//     ImVec2 windowSize = ImGui::GetWindowSize();
//
//     // TODO: wziąć w pętle w zależności od ilości outputów
//     // środek po prawej krawędzi
//     ImVec2 center = ImVec2(windowPos.x + windowSize.x - 15, windowPos.y + windowSize.y * 0.5f);
//
//     // kursor do InvisibleButton tak, żeby kółko dało się kliknąć
//     ImGui::SetCursorScreenPos(ImVec2(center.x - 10, center.y - 10));
//     ImGui::InvisibleButton(("##link" + std::to_string(box.id)).c_str(), ImVec2(20, 20));
//
//     bool isHovered = ImGui::IsItemHovered();
//     bool isClicked = ImGui::IsItemClicked();
//
//     ImDrawList* draw_list = ImGui::GetWindowDrawList();
//
//     // kolor przycisku w zależności od stanu
//     if (box.getNumOutputs() > 0) {
//         ImU32 buttonColor = isClicked ? IM_COL32(255, 0, 0, 255) : (isHovered ? IM_COL32(255, 255, 0, 255) : IM_COL32(200, 200, 0, 255));
//         draw_list->AddCircleFilled(center, 8.0f, buttonColor);
//         draw_list->AddText(ImVec2(center.x - 4, center.y - 7), IM_COL32(0, 0, 0, 255), "+");
//     }
//     // TODO: aż do tąd pętle
//
//
//     // zaczynamy przeciąganie
//     if (isClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
//         if (box.getNumOutputs() > 0)
//             dragging_from = box.id;
//     }
//
//     // zaczynamy przeciąganie
//     if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
//         if (box.getNumOutputs() > 0)
//             dragging_from = box.id;
//     }
//
//     // zbieramy pozycję i czy box jest pod myszą
//     if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
//         is_hovered = true;
//     }
//
//     box.position = ImGui::GetWindowPos();
//
//     // naprawa linii po resize'ie
//     box.size = ImGui::GetWindowSize();
//
//     ImGui::End();
// }

// TODO: przenieść komentarze w wolnej chwili
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

    // flagi okna - pozwól na normalne przeciąganie
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

    if (!ImGui::Begin(title.c_str(), &box.open, flags)) {
        ImGui::GetIO().FontGlobalScale = 1.0f;
        ImGui::End();
        return;
    }

    box.drawContent();

    // rzeczywista pozycję okna
    ImVec2 current_screen_pos = ImGui::GetWindowPos();
    ImVec2 current_screen_size = ImGui::GetWindowSize();

    // czy to okno jest przeciągane?
    bool this_window_dragged = false;

    // czy okno jest aktywne i mysz jest w title bar
    if (ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 title_bar_min = current_screen_pos;
        ImVec2 title_bar_max = ImVec2(current_screen_pos.x + current_screen_size.x, current_screen_pos.y + ImGui::GetFrameHeight());

        if (mouse_pos.x >= title_bar_min.x && mouse_pos.x <= title_bar_max.x &&
            mouse_pos.y >= title_bar_min.y && mouse_pos.y <= title_bar_max.y) {
            this_window_dragged = true;
        }
    }

    // aktualizuj stan przeciągania
    if (this_window_dragged && !isDraggingWindow) {
        isDraggingWindow = true;
        draggedWindowId = box.id;
    } else if (isDraggingWindow && draggedWindowId == box.id && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        isDraggingWindow = false;
        draggedWindowId = -1;
    }

    // punkt połączenia (output)
    ImVec2 center = ImVec2(current_screen_pos.x + current_screen_size.x - 15,
                          current_screen_pos.y + current_screen_size.y * 0.5f);

    ImGui::SetCursorScreenPos(ImVec2(center.x - 10, center.y - 10));
    ImGui::InvisibleButton(("##link" + std::to_string(box.id)).c_str(), ImVec2(20, 20));

    bool isHovered = ImGui::IsItemHovered();
    bool isClicked = ImGui::IsItemClicked();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (box.getNumOutputs() > 0) {
        ImU32 buttonColor = isClicked ? IM_COL32(255, 0, 0, 255) : (isHovered ? IM_COL32(255, 255, 0, 255) : IM_COL32(200, 200, 0, 255));
        draw_list->AddCircleFilled(center, 8.0f, buttonColor);
        draw_list->AddText(ImVec2(center.x - 4, center.y - 7), IM_COL32(0, 0, 0, 255), "+");
    }

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

    // aktualizuj pozycję w przestrzeni świata
    box.position = ImVec2(
        (current_screen_pos.x - viewOffset.x) / zoomAmount,
        (current_screen_pos.y - viewOffset.y) / zoomAmount
    );

    box.size = ImVec2(
        current_screen_size.x / zoomAmount,
        current_screen_size.y / zoomAmount
    );

    ImGui::GetIO().FontGlobalScale = 1.0f;
    ImGui::End();
}

// void guiClass::drawConnections() {
//     ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
//     ImVec2 mousePos = ImGui::GetMousePos();
//
//     for (auto& box : model.getBlocks()) {
//         for (auto it = box->connections.begin(); it != box->connections.end();) {
//             // Szukamy boxa docelowego (target)
//             auto targetIt = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b) { return b->id == *it; });
//             if (targetIt != model.getBlocks().end()) {
//                 // naprawa linii po resize'ie
//                 ImVec2 p1 = (*targetIt)->position;
//                 ImVec2 size1 = (*targetIt)->size;
//                 ImVec2 p2 = box->position;
//                 ImVec2 size2 = box->size;
//
//                 // target: prawa krawędź, środek pionowo
//                 p1.x += size1.x;
//                 p1.y += size1.y * 0.5f;
//                 // source: lewa krawędź, środek pionowo
//                 p2.x += 0;
//                 p2.y += size2.y * 0.5f;
//
//
//                 // sprawdzamy "hoverowanie"
//                 const float detect_radius = 10.0f;
//                 bool hovered = false;
//
//                 // sample line geometry in 20 steps and check distance to mouse
//                 for (int step = 0; step <= 20; ++step) {
//                     float t = step / 20.0f;
//                     ImVec2 pt = BezierCubicCalc(p1, ImVec2(p1.x + 50, p1.y), ImVec2(p2.x - 50, p2.y), p2, t);
//                     if (LengthSqr(pt, mousePos) < detect_radius * detect_radius) {
//                         hovered = true;
//                         break;
//                     }
//                 }
//
//                 // kolor w zależności od "hoverowania"
//                 ImU32 color = hovered ? IM_COL32(255, 100, 100, 255) : IM_COL32(255, 255, 0, 255);
//                 float thickness = hovered ? 5.0f : 3.0f;
//
//                 // rysowanie krzywej
//                 draw_list->AddBezierCubic(
//                     p1,
//                     ImVec2(p1.x + 50, p1.y),
//                     ImVec2(p2.x - 50, p2.y),
//                     p2,
//                     color,
//                     thickness
//                 );
//
//                 // kliknięcie PPM = rozłączenie
//                 if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
//                     it = box->connections.erase(it);
//                     // nie inkrementujemy
//                     continue;
//                 }
//             }
//             ++it;
//         }
//     }
//
//     // rysowanie dynamicznej linki (w trakcie przeciągania)
//     if (dragging_from) {
//         auto it = std::find_if(model.getBlocks().begin(), model.getBlocks().end(), [&](auto& b) { return b->id == *dragging_from; });
//         if (it != model.getBlocks().end()) {
//             // P1 to początek (box źródłowy)
//             ImVec2 p1 = (*it)->position;
//
//             // naprawa linii po resize'ie
//             ImVec2 size = (*it)->size;
//             p1.x += size.x; p1.y += size.y * 0.5f;
//
//             // P2 to koniec (mysz)
//             ImVec2 p2 = ImGui::GetMousePos();
//             draw_list->AddBezierCubic(p1, ImVec2(p1.x + 50, p1.y), ImVec2(p2.x - 50, p2.y), p2, IM_COL32(255, 255, 0, 100), 2.0f);
//         }
//     }
//
//     // próbujemy zakończyć przeciąganie – jeśli mysz została puszczona
//     if (dragging_from && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
//         ImVec2 mousePos = ImGui::GetMousePos();
//         bool connected = false;
//
//         for (auto& box : model.getBlocks()) {
//             ImVec2 boxMin = box->position;
//             ImVec2 boxMax = ImVec2(boxMin.x + 300, boxMin.y + 120);
//
//             if (mousePos.x >= boxMin.x && mousePos.x <= boxMax.x && mousePos.y >= boxMin.y && mousePos.y <= boxMax.y) {
//                 // zmieniamy kierunek połączenia
//                 if (box->id != *dragging_from) {
//                     if (box->getNumInputs() > 0)
//                         box->connections.push_back(*dragging_from);
//                 }
//                 connected = true;
//                 break;
//             }
//         }
//
//         dragging_from = std::nullopt;
//     }
// }

// TODO: przenieść komentarze
void guiClass::drawConnections() {
    // dodaj tę linię na początku
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();

    for (auto& box : model.getBlocks()) {
        for (auto it = box->connections.begin(); it != box->connections.end();) {
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

                draw_list->AddBezierCubic(
                    p1,
                    ImVec2(p1.x + 50 * zoomAmount, p1.y),
                    ImVec2(p2.x - 50 * zoomAmount, p2.y),
                    p2,
                    color,
                    thickness
                );

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

            if (mousePos.x >= boxMin.x && mousePos.x <= boxMax.x &&
                mousePos.y >= boxMin.y && mousePos.y <= boxMax.y) {
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
            if (ImGui::MenuItem("Exit")) { /* zamknij app */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* akcja */ }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) { /* akcja */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            static bool darkMode = true;
            static bool gridEnabled = false;

            ImGui::MenuItem("Dark mode", "", &darkMode);
            ImGui::MenuItem("Show grid", "", &gridEnabled);

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
                model.getBlocks().push_back(std::make_unique<SumBlock>(next_id++));
            if (ImGui::Button("Add Multiply Box"))
                model.getBlocks().push_back(std::make_unique<MultiplyBlock>(next_id++));
            if (ImGui::Button("Add Integrator Box"))
                model.getBlocks().push_back(std::make_unique<IntegratorBlock>(next_id++));
            if (ImGui::Button("Add Diff Box"))
                model.getBlocks().push_back(std::make_unique<DifferentiatorBlock>(next_id++));
            if (ImGui::Button("Add Trigonometric Funcion Box"))
                model.getBlocks().push_back(std::make_unique<TrigonometricFunctionBlock>(next_id++));
            if (ImGui::Button("Add Sqrt Box"))
                model.getBlocks().push_back(std::make_unique<sqrtBlock>(next_id++));
        }

        // modul contorl
        if (ImGui::CollapsingHeader("Control Continous")) {
            if (ImGui::Button("Add Tf box"))
                model.getBlocks().push_back(std::make_unique<TransferFuncionContinous>(next_id++));
            if (ImGui::Button("Add Gain Box"))
                model.getBlocks().push_back(std::make_unique<GainBlock>(next_id++));
            if (ImGui::Button("Add Saturation Box"))
                model.getBlocks().push_back(std::make_unique<SaturationBlock>(next_id++));
            if (ImGui::Button("Add DeadZone Box"))
                model.getBlocks().push_back(std::make_unique<DeadZoneBlock>(next_id++));
        }

        // modul inputy
        if (ImGui::CollapsingHeader("Input")) {
            if (ImGui::Button("Add Step Box"))
                model.getBlocks().push_back(std::make_unique<StepBlock>(next_id++));
            if (ImGui::Button("Add SinusInput Box"))
                model.getBlocks().push_back(std::make_unique<SinusInputBlock>(next_id++));
            if (ImGui::Button("Add PWM Input"))
                model.getBlocks().push_back(std::make_unique<PWMInputBlock>(next_id++));
            if (ImGui::Button("Add WhiteNoise Box"))
                model.getBlocks().push_back(std::make_unique<WhiteNoiseInputBlock>(next_id++));
        }

        // modul print/ploty
        if (ImGui::CollapsingHeader("Print")) {
            if (ImGui::Button("Add Print Box"))
                model.getBlocks().push_back(std::make_unique<PrintBlock>(next_id++));
            if (ImGui::Button("Add Plot Box"))
                model.getBlocks().push_back(std::make_unique<PlotBlock>(next_id++));
            if (ImGui::Button("Add Plot XY Box"))
                model.getBlocks().push_back(std::make_unique<PLotXYBlock>(next_id++));
        }

        // Bloki logicvzne (sprawdzenie czy akhualnia strutra sie do tego nadaje)
        if (ImGui::CollapsingHeader("Logick")) {
            if (ImGui::Button("Add OR Box"))
                model.getBlocks().push_back(std::make_unique<logicORBlock>(next_id++));
            if (ImGui::Button("Add AND Box"))
                model.getBlocks().push_back(std::make_unique<logicANDBlock>(next_id++));
            if (ImGui::Button("Add NOT Box"))
                model.getBlocks().push_back(std::make_unique<logicNOTBlock>(next_id++));
            if (ImGui::Button("Add NOR Box"))
                model.getBlocks().push_back(std::make_unique<logicNORBlock>(next_id++));
        }

        // wysylanie danych
        if (ImGui::CollapsingHeader("Sender")) {
            if (ImGui::Button("Add Sender Box"))
                model.getBlocks().push_back(std::make_unique<DataSenderBlock>(next_id++));
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
        ImGui::InputFloat("Sampling Time", &samplingTime);
        ImGui::InputFloat("Simulation Time", &simTime);

        // combo odpowiedialne za wybor solver i precyzje obliczen (musisz Antek zrobic templeta do tego bo ja nie umim XD)
        static int current_solver = 0;
        const static char* solvers[] = {"RK1", "RK2", "RK4", "RK8", "CN", "GL", "Gear"};

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

        if (simulationRunning) {
            ImGui::BeginDisabled();
            ImGui::Button("Running...");
            ImGui::EndDisabled();
        } else {
            if (ImGui::Button("Run Simulation")) {
                simulationRunning = true;
                // uruchom w osobnym wątku i nie czekaj na niego:
                std::thread([this]() {
                    for (auto& b: model.getBlocks()) {
                        b->setsimTime(this->simTime);
                        b->settimeStep(this->samplingTime);
                    }

                    // cleanup w bloczkach jeśli jest potrzeb
                    model.cleanupBefore();
                    model.makeConnections();
                    // TODO: tu na ogół nie ma być na stałe pętli do 1000
                    // patryk chyba chciał to jakoś ustawiać, idk w sumie nie pamiętam
                    for (int i = 0; i < (this->simTime / this->samplingTime) + 1; i++) {
                        model.simulate();
                    }
                    model.cleanupAfter();
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
