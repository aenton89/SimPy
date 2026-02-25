//
// Created by tajbe on 12.11.2025.
//
#define NOMINMAX
#include "ConnectionManager.h"
#include "../../GUICore.h"




ImVec2 ConnectionManager::worldToScreen(const ImVec2& worldPos) const {
    return {
        worldPos.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
        worldPos.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
    };
}

ImVec2 ConnectionManager::screenToWorld(const ImVec2& screenPos) const {
    return {
        (screenPos.x - guiCore->viewportManager.viewOffset.x) / guiCore->viewportManager.zoomAmount,
        (screenPos.y - guiCore->viewportManager.viewOffset.y) / guiCore->viewportManager.zoomAmount
    };
}

ImVec2 ConnectionManager::getBlockOutputPos(const Block& block) const {
    ImVec2 pos = worldToScreen(block.position);
    ImVec2 size = ImVec2(
        block.size.x * guiCore->viewportManager.zoomAmount,
        block.size.y * guiCore->viewportManager.zoomAmount
    );
    // TODO: narazie środek prawej krawędzi - ale jakby był wybór output portów to by się zmieniło
    return {pos.x + size.x, pos.y + size.y * 0.5f};
}

ImVec2 ConnectionManager::getBlockInputPos(const Block& block) const {
    ImVec2 pos = worldToScreen(block.position);
    ImVec2 size = ImVec2(
        block.size.x * guiCore->viewportManager.zoomAmount,
        block.size.y * guiCore->viewportManager.zoomAmount
    );
    // TODO: narazie środek lewej krawędzi - ale jakby był wybór input portów to by się zmieniło
    return {pos.x, pos.y + size.y * 0.5f};
}

void ConnectionManager::drawConnections() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 mousePosWorld = screenToWorld(mousePos);

    // rysuj wszystkie istniejące połączenia
    for (auto& conn : guiCore->model.getConnections()) {
        drawConnection(conn, drawList);
    }

    // rysuj draft (tworzone połączenie)
    if (currentDraft.isActive())
        drawDraftConnection(drawList);

    // obsługa przeciągania węzłów
    handleNodeDragging(mousePosWorld);

    // obsługa tworzenia nowych połączeń
    if (currentDraft.isActive())
        handleConnectionCreation(mousePos);
}

void ConnectionManager::drawConnection(Connection& conn, ImDrawList* drawList) {
    if (!conn.isValid())
        return;

    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 mousePosWorld = screenToWorld(mousePos);

    // pozycje końców
    ImVec2 startPos = getBlockOutputPos(*conn.sourceBlock);
    ImVec2 endPos = getBlockInputPos(*conn.targetBlock);

    // zbieranie wszystkich punktów krzywej: start -> węzły -> koniec
    std::vector<ImVec2> pathPoints;
    pathPoints.push_back(startPos);
    for (const auto& node : conn.controlNodes) {
        pathPoints.push_back(worldToScreen(node));
    }
    pathPoints.push_back(endPos);

    // sprawdzenie hover'a na krzywej
    float threshold = drawSettings.pathDetectionRadius * guiCore->viewportManager.zoomAmount;
    bool pathHovered = useOrthogonalLines ? isPointNearOrthogonalPath(mousePos, pathPoints, threshold) : isPointNearCurvePath(mousePos, pathPoints, threshold);

    // rysowanie krzywej i węzłów
    drawConnectionPath(pathPoints, pathHovered, drawList);
    if (!conn.controlNodes.empty())
        drawControlNodes(conn, mousePos, drawList);

    // i na koniec obsługa interakcji
    handleConnectionDeletion(conn, pathHovered);
    handleNodeAddition(conn, pathHovered, mousePosWorld);
}

void ConnectionManager::drawConnectionPath(const std::vector<ImVec2>& points, bool hovered, ImDrawList* drawList) const {
    if (points.size() < 2)
        return;

    float thickness = (hovered ? drawSettings.hoveredThickness : drawSettings.normalThickness) * guiCore->viewportManager.zoomAmount;
    ImU32 color = hovered ? UIStyles::CONNECTION_HOVER_COLOR : UIStyles::CONNECTION_COLOR;

    if (useOrthogonalLines) {
        float capRadius = thickness * 0.5f;

        // helpery - odcinki ortogonalne i kółka w narożnikach
        auto addOrthoLine = [&](ImVec2 a, ImVec2 b) {
            drawList->AddLine(a, b, color, thickness);
        };
        auto addCap = [&](ImVec2 p) {
            drawList->AddCircleFilled(p, capRadius, color);
        };

        // brak węzłów kontrolnych - H->V->H z pionowym odcinkiem w midX
        if (points.size() == 2) {
            ImVec2 s = points[0];
            ImVec2 e = points[1];
            float midX = (s.x + e.x) * 0.5f;
            ImVec2 c1 = {midX, s.y};
            ImVec2 c2 = {midX, e.y};

            addOrthoLine(s, c1);
            addCap(c1);
            addOrthoLine(c1, c2);
            addOrthoLine(c2, e);
            addCap(c2);
        } else {
            // węzły kontrolne - schemat jak komentarz nad isPointNearOrthogonalPath
            const size_t last = points.size() - 1;

            // segment start -> w1
            {
                ImVec2 s = points[0];
                ImVec2 w1 = points[1];
                ImVec2 c = {w1.x, s.y};

                addOrthoLine(s, c);
                addCap(c);
                addOrthoLine(c, w1);
            }

            // segmenty wi -> wi+1 (węzeł do węzła)
            for (size_t i = 1; i + 1 < last; ++i) {
                ImVec2 wa = points[i];
                ImVec2 wb = points[i + 1];
                ImVec2 c = {wb.x, wa.y};

                addOrthoLine(wa, c);
                addCap(c);
                addOrthoLine(c, wb);
            }

            // segment wN -> end
            {
                ImVec2 prev = points[last - 2];
                ImVec2 wN = points[last - 1];
                ImVec2 e = points[last];
                float lo = std::min(prev.y, e.y);
                float hi = std::max(prev.y, e.y);

                // wN w czy poza pasem - H -> V
                ImVec2 c = (wN.y >= lo && wN.y <= hi) ? ImVec2(wN.x, e.y) : ImVec2(e.x, wN.y);

                addOrthoLine(wN, c);
                addCap(c);
                addOrthoLine(c, e);
            }
        }
    } else {
        float offset = drawSettings.bezierControlOffset * guiCore->viewportManager.zoomAmount;

        // rysuj segmenty krzywej Bezier'a
        for (size_t i = 0; i + 1 < points.size(); ++i) {
            ImVec2 c1 = ImVec2(points[i].x + offset, points[i].y);
            ImVec2 c2 = ImVec2(points[i+1].x - offset, points[i+1].y);

            drawList->AddBezierCubic(
                points[i], c1, c2, points[i+1],
                color, thickness
            );
        }
    }
}

void ConnectionManager::drawControlNodes(Connection& conn, const ImVec2& mousePos, ImDrawList* drawList) {
    float radius = drawSettings.nodeRadius * guiCore->viewportManager.zoomAmount;

    for (size_t i = 0; i < conn.controlNodes.size(); ++i) {
        ImVec2 nodeScreenPos = worldToScreen(conn.controlNodes[i]);

        // sprawdzanie hover'a
        float distSq = (nodeScreenPos.x - mousePos.x) * (nodeScreenPos.x - mousePos.x) +
                        (nodeScreenPos.y - mousePos.y) * (nodeScreenPos.y - mousePos.y);
        bool hovered = distSq < (radius * 3.0f) * (radius * 3.0f);

        ImU32 color = hovered ? UIStyles::CONTROL_NODE_HOVER : UIStyles::CONTROL_NODE_NORMAL;

        // rysuj węzeł
        drawList->AddCircleFilled(nodeScreenPos, radius, color);
        drawList->AddCircle(nodeScreenPos, radius, UIStyles::CONTROL_NODE_BORDER, 12, 2.0f);

        // obsługa kliknięcia - rozpocznij przeciąganie
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !draggingNode.isActive()) {
            draggingNode.connection = &conn;
            draggingNode.nodeIndex = i;
        }

        // PPM - usuń węzeł
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            conn.controlNodes.erase(conn.controlNodes.begin() + i);
            // wektor zmieniony, więc koniec
            return;
        }
    }
}

void ConnectionManager::handleNodeDragging(const ImVec2& mousePosWorld) {
    if (!draggingNode.isActive())
        return;

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        // aktualizuj pozycję węzła
        if (draggingNode.nodeIndex < draggingNode.connection->controlNodes.size())
            draggingNode.connection->controlNodes[draggingNode.nodeIndex] = mousePosWorld;
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        draggingNode.reset();
}

void ConnectionManager::startConnectionDraft(int blockId, int portIndex) {
    currentDraft.sourceBlockId = blockId;
    currentDraft.sourcePort = portIndex;
}

void ConnectionManager::cancelConnectionDraft() {
    currentDraft.reset();
}

void ConnectionManager::drawDraftConnection(ImDrawList* drawList) const {
    auto sourceBlock = guiCore->model.findBlockById(currentDraft.sourceBlockId);

    if (!sourceBlock)
        return;

    ImVec2 startPos = getBlockOutputPos(*sourceBlock);
    ImVec2 endPos = ImGui::GetMousePos();
    float thickness = 2.0f * guiCore->viewportManager.zoomAmount;

    if (useOrthogonalLines) {
        float midX = (startPos.x + endPos.x) * 0.5f;

        ImVec2 corner1 = ImVec2(midX, startPos.y);
        ImVec2 corner2 = ImVec2(midX, endPos.y);

        drawList->AddLine(startPos, ImVec2(midX, startPos.y), UIStyles::CONNECTION_DRAGGING_COLOR, thickness);
        drawList->AddLine(ImVec2(midX, startPos.y), ImVec2(midX, endPos.y), UIStyles::CONNECTION_DRAGGING_COLOR, thickness);
        drawList->AddLine(ImVec2(midX, endPos.y), endPos, UIStyles::CONNECTION_DRAGGING_COLOR, thickness);

        float capRadius = thickness * 0.5f;
        drawList->AddCircleFilled(corner1, capRadius, UIStyles::CONNECTION_DRAGGING_COLOR);
        drawList->AddCircleFilled(corner2, capRadius, UIStyles::CONNECTION_DRAGGING_COLOR);
    } else {
        float offset = drawSettings.bezierControlOffset * guiCore->viewportManager.zoomAmount;
        ImVec2 c1 = ImVec2(startPos.x + offset, startPos.y);
        ImVec2 c2 = ImVec2(endPos.x - offset, endPos.y);

        drawList->AddBezierCubic(
            startPos, c1, c2, endPos,
            UIStyles::CONNECTION_DRAGGING_COLOR,
            thickness
        );
    }
}

void ConnectionManager::handleConnectionCreation(const ImVec2& mousePos) {
    // puszczono LPM - próba połączenia
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (auto sourceBlock = guiCore->model.findBlockById(currentDraft.sourceBlockId)) {
            // czy kursor jest nad jakimś blokiem
            for (auto& targetBlock : guiCore->model.getBlocks()) {
                if (targetBlock->id == currentDraft.sourceBlockId)
                    continue;

                ImVec2 blockMin = worldToScreen(targetBlock->position);
                ImVec2 blockSize = ImVec2(
                    targetBlock->size.x * guiCore->viewportManager.zoomAmount,
                    targetBlock->size.y * guiCore->viewportManager.zoomAmount
                );
                ImVec2 blockMax = ImVec2(
                    blockMin.x + blockSize.x,
                    blockMin.y + blockSize.y
                );

                // sprawdź hitbox
                if (mousePos.x >= blockMin.x && mousePos.x <= blockMax.x && mousePos.y >= blockMin.y && mousePos.y <= blockMax.y) {
                    // TODO: Dodać UI do wyboru konkretnego portu
                    // narazie - znajdź pierwszy wolny port
                    int targetPort = -1;
                    for (int i = 0; i < targetBlock->getNumInputs(); i++) {
                        if (!guiCore->model.isInputPortUsed(targetBlock, i)) {
                            targetPort = i;
                            break;
                        }
                    }

                    // jeśli nie znaleziono wolnego portu, pomiń
                    if (targetPort == -1) {
                        std::cerr << "No free input ports available\n";
                        break;
                    }

                    // próbuj dodać połączenie - addConnection() wykonuje walidacje
                    guiCore->model.addConnection(
                        sourceBlock, currentDraft.sourcePort,
                        targetBlock, targetPort
                    );

                    break;
                }
            }
        }

        currentDraft.reset();
    }
}

void ConnectionManager::handleConnectionDeletion(const Connection& conn, bool pathHovered) const {
    if (pathHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        guiCore->model.removeConnection(
            conn.sourceBlock, conn.sourcePort,
            conn.targetBlock, conn.targetPort
        );
    }
}

void ConnectionManager::removeConnectionsForBlocks(const std::set<int>& blockIds) const {
    auto& connections = guiCore->model.getConnections();

    for (auto it = connections.begin(); it != connections.end();) {
        if ((it->sourceBlock && blockIds.contains(it->sourceBlock->id)) || (it->targetBlock && blockIds.contains(it->targetBlock->id))) {
            guiCore->model.removeConnection(it->sourceBlock, it->sourcePort, it->targetBlock, it->targetPort);
            it = connections.begin();
        } else {
            ++it;
        }
    }
}

void ConnectionManager::handleNodeAddition(Connection& conn, bool pathHovered, const ImVec2& mousePosWorld) {
    if (pathHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        conn.controlNodes.push_back(mousePosWorld);
}

bool ConnectionManager::isPointNearCurvePath(const ImVec2& point, const std::vector<ImVec2>& pathPoints, float threshold) const {
    if (pathPoints.size() < 2)
        return false;

    float offset = drawSettings.bezierControlOffset * guiCore->viewportManager.zoomAmount;

    for (size_t i = 0; i + 1 < pathPoints.size(); ++i) {
        ImVec2 c1 = ImVec2(pathPoints[i].x + offset, pathPoints[i].y);
        ImVec2 c2 = ImVec2(pathPoints[i+1].x - offset, pathPoints[i+1].y);

        // próbkuj krzywą Beziera
        for (int step = 0; step <= 30; ++step) {
            float t = step / 30.0f;

            // oblicz punkt na krzywej
            float u = 1.0f - t;
            float tt = t * t;
            float uu = u * u;
            float uuu = uu * u;
            float ttt = tt * t;

            ImVec2 pt;
            pt.x = uuu * pathPoints[i].x + 3 * uu * t * c1.x + 3 * u * tt * c2.x + ttt * pathPoints[i+1].x;
            pt.y = uuu * pathPoints[i].y + 3 * uu * t * c1.y + 3 * u * tt * c2.y + ttt * pathPoints[i+1].y;

            float distSq = (pt.x - point.x) * (pt.x - point.x) + (pt.y - point.y) * (pt.y - point.y);

            if (distSq < threshold * threshold)
                return true;
        }
    }

    return false;
}

/* tldr:
* start -> w1: start -> h -> (w1.x, start.y) -> v -> w1
* wi -> wi+1: wa -> h -> (wb.x, wa.y) -> v -> wb
* wn -> end: wn -> v -> (wn.x, end.y) -> h -> end
*
* oraz te warunki:
* gdy mamy chociaż 1 wezel to (w przypadku gdy jest tylko jeden to last-1 = start)
* jeśli last.y w przedziale (last-1.y, end.y) lub (end.y, last-1.y) to ok, zostaje jak jest
* jeśli last.y jest wyżej/niżej niz end.y i last-1.y to
* last - > h -> (end.x, last.y) -> v -> end
*/
bool ConnectionManager::isPointNearOrthogonalPath(const ImVec2& point, const std::vector<ImVec2>& pathPoints, float threshold) const {
    if (pathPoints.size() < 2)
        return false;

    // helpery - dystans punktu od odcinka p0->p1 i sprawdzenie czy jest blisko
    auto distToSegment = [](ImVec2 p, ImVec2 a, ImVec2 b) -> float {
        float dx = b.x - a.x, dy = b.y - a.y;
        float lenSq = dx * dx + dy * dy;
        float t = lenSq > 0.0f ? ((p.x - a.x) * dx + (p.y - a.y) * dy) / lenSq : 0.0f;
        t = std::max(0.0f, std::min(1.0f, t));
        float cx = a.x + t * dx, cy = a.y + t * dy;
        float ex = p.x - cx, ey = p.y - cy;

        return std::sqrt(ex * ex + ey * ey);
    };

    auto nearPoint = [&](ImVec2 a, ImVec2 b) -> bool{
        return distToSegment(point, a, b) < threshold;
    };

    // brak węzłów - H->V->H z midX
    if (pathPoints.size() == 2) {
        ImVec2 s = pathPoints[0];
        ImVec2 e = pathPoints[1];
        float midX = (s.x + e.x) * 0.5f;
        ImVec2 c1 = {midX, s.y};
        ImVec2 c2 = {midX, e.y};

        if (nearPoint(s, c1) || nearPoint(c1, c2) || nearPoint(c2, e))
            return true;
    } else {
        // spójne z drawConnectionPath
        const size_t last = pathPoints.size() - 1;

        // start -> w1
        {
            ImVec2 s = pathPoints[0];
            ImVec2 w1 = pathPoints[1];
            ImVec2 c = {w1.x, s.y};

            if (nearPoint(s, c) || nearPoint(c, w1))
                return true;
        }

        // wi -> wi+1
        for (size_t i = 1; i + 1 < last; ++i) {
            ImVec2 wa = pathPoints[i];
            ImVec2 wb = pathPoints[i + 1];
            ImVec2 c = {wb.x, wa.y};

            if (nearPoint(wa, c) || nearPoint(c, wb))
                return true;
        }

        // wN -> end
        {
            ImVec2 prev = pathPoints[last - 2];
            ImVec2 wN = pathPoints[last - 1];
            ImVec2 e = pathPoints[last];
            float lo = std::min(prev.y, e.y);
            float hi = std::max(prev.y, e.y);

            // wN w czy poza pasem - H -> V
            ImVec2 c = (wN.y >= lo && wN.y <= hi) ? ImVec2(wN.x, e.y) : ImVec2(e.x, wN.y);


            if (nearPoint(wN, c) || nearPoint(c, e))
                return true;
        }
    }

    return false;
}

bool ConnectionManager::isDraftingConnection() const {
    return currentDraft.isActive();
}

bool ConnectionManager::isDraggingNode() const {
    return draggingNode.isActive();
}