//
// Created by tajbe on 12.11.2025.
//
#include "ConnectionManager.h"
#include "../GUICore.h"



void ConnectionManager::setGUICore(GUICore *gui) {
	guiCore = gui;
}

void ConnectionManager::drawConnections() {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();

    for (auto& box : guiCore->model.getBlocks()) {
        for (auto it = box->connections.begin(); it != box->connections.end();) {
            // Szukamy boxa docelowego (target)
            auto targetIt = std::find_if(guiCore->model.getBlocks().begin(), guiCore->model.getBlocks().end(), [&](auto& b) { return b->id == *it; });
            if (targetIt != guiCore->model.getBlocks().end()) {
                // oblicz pozycje w przestrzeni ekranu (po transformacji)
                ImVec2 target_pos = ImVec2(
                    (*targetIt)->position.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
                    (*targetIt)->position.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
                );
                ImVec2 target_size = ImVec2(
                    (*targetIt)->size.x * guiCore->viewportManager.zoomAmount,
                    (*targetIt)->size.y * guiCore->viewportManager.zoomAmount
                );

                ImVec2 source_pos = ImVec2(
                    box->position.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
                    box->position.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
                );
                ImVec2 source_size = ImVec2(
                    box->size.x * guiCore->viewportManager.zoomAmount,
                    box->size.y * guiCore->viewportManager.zoomAmount
                );

                // punkty połączenia
                ImVec2 p1 = ImVec2(target_pos.x + target_size.x, target_pos.y + target_size.y * 0.5f);
                ImVec2 p2 = ImVec2(source_pos.x, source_pos.y + source_size.y * 0.5f);

                // sprawdź hover na linii
                const float detect_radius = 10.0f * guiCore->viewportManager.zoomAmount;
                bool hovered = false;

                // sample line geometry in 20 steps and check distance to mouse
                for (int step = 0; step <= 20; ++step) {
                    float t = step / 20.0f;
                    ImVec2 pt = BezierCubicCalc(p1, ImVec2(p1.x + 50 * guiCore->viewportManager.zoomAmount, p1.y), ImVec2(p2.x - 50 * guiCore->viewportManager.zoomAmount, p2.y), p2, t);
                    if (LengthSqr(pt, mousePos) < detect_radius * detect_radius) {
                        hovered = true;
                        break;
                    }
                }

                // rysowanie z uwzględnieniem skali
                ImU32 color = hovered ? IM_COL32(255, 100, 100, 255) : IM_COL32(255, 255, 0, 255);
                float thickness = (hovered ? 5.0f : 3.0f) * guiCore->viewportManager.zoomAmount;

                // rysowanie krzywej
                draw_list->AddBezierCubic(
                    p1,
                    ImVec2(p1.x + 50 * guiCore->viewportManager.zoomAmount, p1.y),
                    ImVec2(p2.x - 50 * guiCore->viewportManager.zoomAmount, p2.y),
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
        auto it = std::find_if(guiCore->model.getBlocks().begin(), guiCore->model.getBlocks().end(), [&](auto& b) { return b->id == *dragging_from; });
        if (it != guiCore->model.getBlocks().end()) {
            ImVec2 source_pos = ImVec2(
                (*it)->position.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
                (*it)->position.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
            );
            ImVec2 source_size = ImVec2(
                (*it)->size.x * guiCore->viewportManager.zoomAmount,
                (*it)->size.y * guiCore->viewportManager.zoomAmount
            );

            ImVec2 p1 = ImVec2(source_pos.x + source_size.x, source_pos.y + source_size.y * 0.5f);
            // P2 to koniec (mysz)
            ImVec2 p2 = ImGui::GetMousePos();

            draw_list->AddBezierCubic(
                p1,
                ImVec2(p1.x + 50 * guiCore->viewportManager.zoomAmount, p1.y),
                ImVec2(p2.x - 50 * guiCore->viewportManager.zoomAmount, p2.y),
                p2,
                IM_COL32(255, 255, 0, 100),
                2.0f * guiCore->viewportManager.zoomAmount
            );
        }
    }

    // zakończenie przeciągania z uwzględnieniem transformacji
    if (dragging_from && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        bool connected = false;

        for (auto& box : guiCore->model.getBlocks()) {
            // przekształć pozycję boxa do przestrzeni ekranu
            ImVec2 transformed_pos = ImVec2(
                box->position.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
                box->position.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
            );
            ImVec2 transformed_size = ImVec2(
                box->size.x * guiCore->viewportManager.zoomAmount,
                box->size.y * guiCore->viewportManager.zoomAmount
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