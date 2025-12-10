//
// Created by tajbe on 12.11.2025.
//
#include "ConnectionManager.h"
#include "../../GUICore.h"



ImVec2 ConnectionManager::worldToScreen(const ImVec2& worldPos) const {
    // konwersja pozycji ze świata (stała skala) do pozycji ekranu (powiększenie + przesunięcie widoku)
    return {
        worldPos.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
        worldPos.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
    };
}

ImVec2 ConnectionManager::screenToWorld(const ImVec2& screenPos) const {
    // konwersja pozycji ekranu do współrzędnych świata (odwrócenie zoom + offset)
    return {
        (screenPos.x - guiCore->viewportManager.viewOffset.x) / guiCore->viewportManager.zoomAmount,
        (screenPos.y - guiCore->viewportManager.viewOffset.y) / guiCore->viewportManager.zoomAmount
    };
}

void ConnectionManager::drawSingleConnection(Connection& conn, const ImVec2& mousePos, const ImVec2& mousePosWorld, ImDrawList* drawList) {
    // jeśli połączenie nie ma źródła albo celu — nic nie rysujemy
    if (!conn.sourceBlock || !conn.targetBlock)
        return;

    // --- OBLICZANIE POZYCJI BLOKÓW NA EKRANIE ---

    // pozycja i rozmiar bloku źródłowego
    ImVec2 source_pos = worldToScreen(conn.sourceBlock->position);
    ImVec2 source_size = ImVec2(
        conn.sourceBlock->size.x * guiCore->viewportManager.zoomAmount,
        conn.sourceBlock->size.y * guiCore->viewportManager.zoomAmount
    );

    // pozycja i rozmiar bloku docelowego
    ImVec2 target_pos = worldToScreen(conn.targetBlock->position);
    ImVec2 target_size = ImVec2(
        conn.targetBlock->size.x * guiCore->viewportManager.zoomAmount,
        conn.targetBlock->size.y * guiCore->viewportManager.zoomAmount
    );

    // punkt wyjściowy krzywej = środek prawej krawędzi bloku źródłowego
    ImVec2 p1 = ImVec2(source_pos.x + source_size.x, source_pos.y + source_size.y * 0.5f);

    // punkt końcowy krzywej = środek lewej krawędzi bloku docelowego
    ImVec2 p2 = ImVec2(target_pos.x, target_pos.y + target_size.y * 0.5f);

    // --- RYSOWANIE WĘZŁÓW KONTROLNYCH (custom handles) ---
    bool anyNodeHovered = false;

    for (size_t i = 0; i < conn.controlNodes.size(); ++i) {
        constexpr float nodeRadius = 6.0f;

        // ekranowa pozycja węzła
        ImVec2 nodeScreenPos = worldToScreen(conn.controlNodes[i]);

        // sprawdzanie hover
        bool nodeHovered = LengthSqr(nodeScreenPos, mousePos) < (nodeRadius * 3.0f) * (nodeRadius * 3.0f);

        if (nodeHovered)
            anyNodeHovered = true;

        // kolor zależny od hover
        ImU32 nodeColor = nodeHovered ? IM_COL32(255, 150, 0, 255) : IM_COL32(200, 200, 200, 200);

        float visualRadius = nodeRadius * guiCore->viewportManager.zoomAmount;

        // rysowanie koła węzła
        drawList->AddCircleFilled(nodeScreenPos, visualRadius, nodeColor);
        drawList->AddCircle(nodeScreenPos, visualRadius, IM_COL32(50, 50, 50, 255), 12, 2.0f);

        // kliknięcie LPM — rozpoczęcie przeciągania węzła
        if (nodeHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !draggingNode)
            draggingNode = DraggingNode{&conn, static_cast<int>(i)};

        // kliknięcie PPM — usunięcie węzła
        if (nodeHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            conn.controlNodes.erase(conn.controlNodes.begin() + i);
            return; // nie rysujemy dalej — wektor zmieniony
        }
    }

    // --- HOVER NA KRZYWEJ GŁÓWNEJ (jeśli nie są hoverowane węzły) ---
    const float detectRadius = 10.0f * guiCore->viewportManager.zoomAmount;
    bool curveHovered = false;

    // definiowanie kontrolnych punktów krzywej bazowej
    ImVec2 c1 = ImVec2(p1.x + 50 * guiCore->viewportManager.zoomAmount, p1.y);
    ImVec2 c2 = ImVec2(p2.x - 50 * guiCore->viewportManager.zoomAmount, p2.y);

    // sprawdzanie czy kursor jest blisko krzywej
    for (int step = 0; step <= 30; ++step) {
        float t = step / 30.0f;
        ImVec2 pt = BezierCubicCalc(p1, c1, c2, p2, t);
        if (LengthSqr(pt, mousePos) < detectRadius * detectRadius) {
            curveHovered = true;
            break;
        }
    }

    // kolor i grubość krzywej zależne od hover
    ImU32 curveColor = (curveHovered && !anyNodeHovered) ? IM_COL32(255, 100, 100, 255) : IM_COL32(255, 255, 0, 255);
    float thickness = ((curveHovered && !anyNodeHovered) ? 5.0f : 3.0f) * guiCore->viewportManager.zoomAmount;

    // --- RYSOWANIE CAŁEJ KRZYWEJ Z DODATKOWYMI WĘZŁAMI ---

    // zbieramy wszystkie punkty po kolei: start → węzły kontrolne → koniec
    std::vector<ImVec2> points;
    points.push_back(p1);
    for (auto& node : conn.controlNodes)
        points.push_back(worldToScreen(node));
    points.push_back(p2);

    // rysowanie segmentów krzywych pomiędzy punktami
    for (size_t i = 0; i + 1 < points.size(); ++i) {
        drawList->AddBezierCubic(
            points[i],
            ImVec2(points[i].x + 50 * guiCore->viewportManager.zoomAmount, points[i].y),
            ImVec2(points[i+1].x - 50 * guiCore->viewportManager.zoomAmount, points[i+1].y),
            points[i+1],
            curveColor,
            thickness
        );
    }

    // --- DODAWANIE WĘZŁA PODWÓJNYM KLIKNIĘCIEM ---
    if (curveHovered && !anyNodeHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        conn.controlNodes.push_back(mousePosWorld);
    }

    // --- PPM NA KRZYWEJ — USUWANIE POŁĄCZENIA ---
    if (curveHovered && !anyNodeHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {

        // usuwanie z listy globalnych połączeń
        auto& connections = guiCore->model.getConnections();
        std::erase_if(connections, [&](const Connection& c) {
                          return c.sourceBlock == conn.sourceBlock && c.targetBlock == conn.targetBlock;
                      });

        // usuwanie z listy lokalnej bloku
        if (conn.sourceBlock) {
            auto& sourceConns = conn.sourceBlock->connections;
            std::erase(sourceConns, conn.targetBlock->id);
        }
    }
}

void ConnectionManager::handleNodeDragging(const ImVec2& mousePosWorld) {

    // kiedy trzymamy LPM — przeciągamy aktywny węzeł
    if (draggingNode && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

        // aktualizacja pozycji węzła w świecie
        if (draggingNode->connection &&
            draggingNode->nodeIndex < draggingNode->connection->controlNodes.size()) {

            draggingNode->connection->controlNodes[draggingNode->nodeIndex] = mousePosWorld;
        }
    }

    // po puszczeniu LPM — kończymy przeciąganie
    if (draggingNode && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
        draggingNode.reset();
}

void ConnectionManager::handleConnectionCreation(const ImVec2& mousePos, ImDrawList* drawList) {

    // --- RYSOWANIE PODGLĄDU TWORZONEGO POŁĄCZENIA ---

    if (draggingFrom) {

        // szukamy bloku, z którego zaczęto przeciągać
        auto it = std::ranges::find_if(guiCore->model.getBlocks(),[&](auto& b) {
            return b->id == *draggingFrom;
        });

        if (it != guiCore->model.getBlocks().end()) {

            // oblicz ekranowe pozycje punktu startowego
            ImVec2 source_pos = worldToScreen((*it)->position);
            ImVec2 source_size = ImVec2(
                (*it)->size.x * guiCore->viewportManager.zoomAmount,
                (*it)->size.y * guiCore->viewportManager.zoomAmount
            );

            ImVec2 p1 = ImVec2(source_pos.x + source_size.x, source_pos.y + source_size.y * 0.5f);
            ImVec2 p2 = mousePos; // bieżąca pozycja kursora

            // rysujemy poglądową linię
            drawList->AddBezierCubic(
                p1,
                ImVec2(p1.x + 50 * guiCore->viewportManager.zoomAmount, p1.y),
                ImVec2(p2.x - 50 * guiCore->viewportManager.zoomAmount, p2.y),
                p2,
                IM_COL32(255, 255, 0, 100),
                2.0f * guiCore->viewportManager.zoomAmount
            );
        }
    }

    // --- PUSZCZENIE LPM — PRÓBUJEMY ZŁĄCZYĆ Z BLOKIEM ---

    if (draggingFrom && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {

        // iterujemy po wszystkich blokach
        for (auto& box : guiCore->model.getBlocks()) {

            // ekranowy bounding box
            ImVec2 boxMin = worldToScreen(box->position);
            ImVec2 boxSize = ImVec2(
                box->size.x * guiCore->viewportManager.zoomAmount,
                box->size.y * guiCore->viewportManager.zoomAmount
            );
            ImVec2 boxMax = ImVec2(boxMin.x + boxSize.x, boxMin.y + boxSize.y);

            // sprawdzanie hitboxa
            if (mousePos.x >= boxMin.x && mousePos.x <= boxMax.x &&
                mousePos.y >= boxMin.y && mousePos.y <= boxMax.y) {

                // upewniamy się, że łączymy dwa różne bloki
                if (box->id != *draggingFrom && box->getNumInputs() > 0) {

                    // dodajemy połączenie do listy wejść bloku docelowego
                    box->connections.push_back(*draggingFrom);

                    // tworzymy globalne połączenie w modelu
                    if (auto source = guiCore->model.findBlockById(*draggingFrom)) {
                        Connection newConn;
                        newConn.sourceBlock = source;
                        newConn.targetBlock = box;
                        guiCore->model.getConnections().push_back(newConn);
                    }
                }

                break;
            }
        }

        // resetujemy stan przeciągania
        draggingFrom.reset();
    }
}

void ConnectionManager::drawConnections() {

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // pozycja myszy — ekran i świat
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 mousePosWorld = screenToWorld(mousePos);

    // --- RYSOWANIE WSZYSTKICH ISTNIEJĄCYCH POŁĄCZEŃ ---
    for (auto& conn : guiCore->model.getConnections()) {
        drawSingleConnection(conn, mousePos, mousePosWorld, drawList);
    }

    // --- OBSŁUGA PRZECIĄGANIA WĘZŁÓW ---
    handleNodeDragging(mousePosWorld);

    // --- OBSŁUGA TWORZENIA NOWYCH POŁĄCZEŃ ---
    handleConnectionCreation(mousePos, drawList);
}