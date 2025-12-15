//
// Created by tajbe on 12.11.2025.
//
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
    std::vector<ImVec2> curvePoints;
    curvePoints.push_back(startPos);
    for (const auto& node : conn.controlNodes) {
        curvePoints.push_back(worldToScreen(node));
    }
    curvePoints.push_back(endPos);

    // sprawdzenie hover'a na krzywej
    float threshold = drawSettings.curveDetectionRadius * guiCore->viewportManager.zoomAmount;
    bool curveHovered = isPointNearCurve(mousePos, curvePoints, threshold);

    // rysowanie krzywej i węzłów
    drawConnectionCurve(curvePoints, curveHovered, drawList);
    if (!conn.controlNodes.empty())
        drawControlNodes(conn, mousePos, drawList);

    // i na koniec obsługa interakcji
    handleConnectionDeletion(conn, curveHovered);
    handleNodeAddition(conn, curveHovered, mousePosWorld);
}

void ConnectionManager::drawConnectionCurve(const std::vector<ImVec2>& points, bool hovered, ImDrawList* drawList) const {
    if (points.size() < 2)
        return;

    float thickness = (hovered ? drawSettings.hoveredThickness : drawSettings.normalThickness) * guiCore->viewportManager.zoomAmount;
    ImU32 color = hovered ? drawSettings.hoveredColor : drawSettings.normalColor;
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

void ConnectionManager::drawControlNodes(Connection& conn, const ImVec2& mousePos, ImDrawList* drawList) {
    float radius = drawSettings.nodeRadius * guiCore->viewportManager.zoomAmount;

    for (size_t i = 0; i < conn.controlNodes.size(); ++i) {
        ImVec2 nodeScreenPos = worldToScreen(conn.controlNodes[i]);

        // sprawdzanie hover'a
        float distSq = (nodeScreenPos.x - mousePos.x) * (nodeScreenPos.x - mousePos.x) +
                      (nodeScreenPos.y - mousePos.y) * (nodeScreenPos.y - mousePos.y);
        bool hovered = distSq < (radius * 3.0f) * (radius * 3.0f);

        ImU32 color = hovered ? drawSettings.nodeHoveredColor : drawSettings.nodeNormalColor;

        // rysuj węzeł
        drawList->AddCircleFilled(nodeScreenPos, radius, color);
        drawList->AddCircle(nodeScreenPos, radius, IM_COL32(50, 50, 50, 255), 12, 2.0f);

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

    float offset = drawSettings.bezierControlOffset * guiCore->viewportManager.zoomAmount;

    ImVec2 c1 = ImVec2(startPos.x + offset, startPos.y);
    ImVec2 c2 = ImVec2(endPos.x - offset, endPos.y);

    drawList->AddBezierCubic(
        startPos, c1, c2, endPos,
        drawSettings.draftColor,
        2.0f * guiCore->viewportManager.zoomAmount
    );
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

void ConnectionManager::handleConnectionDeletion(const Connection& conn, bool curveHovered) const {
    if (curveHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
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

void ConnectionManager::handleNodeAddition(Connection& conn, bool curveHovered, const ImVec2& mousePosWorld) {
    if (curveHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        conn.controlNodes.push_back(mousePosWorld);
}

bool ConnectionManager::isPointNearCurve(
    const ImVec2& point,
    const std::vector<ImVec2>& curvePoints,
    float threshold) const
{
    if (curvePoints.size() < 2)
        return false;

    float offset = drawSettings.bezierControlOffset * guiCore->viewportManager.zoomAmount;

    for (size_t i = 0; i + 1 < curvePoints.size(); ++i) {
        ImVec2 c1 = ImVec2(curvePoints[i].x + offset, curvePoints[i].y);
        ImVec2 c2 = ImVec2(curvePoints[i+1].x - offset, curvePoints[i+1].y);

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
            pt.x = uuu * curvePoints[i].x + 3 * uu * t * c1.x + 3 * u * tt * c2.x + ttt * curvePoints[i+1].x;
            pt.y = uuu * curvePoints[i].y + 3 * uu * t * c1.y + 3 * u * tt * c2.y + ttt * curvePoints[i+1].y;

            float distSq = (pt.x - point.x) * (pt.x - point.x) + (pt.y - point.y) * (pt.y - point.y);

            if (distSq < threshold * threshold)
                return true;
        }
    }

    return false;
}

bool ConnectionManager::isDraftingConnection() const {
    return currentDraft.isActive();
}