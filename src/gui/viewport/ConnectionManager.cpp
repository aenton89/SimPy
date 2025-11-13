//
// Created by tajbe on 12.11.2025.
//
#include "ConnectionManager.h"
#include "../GUICore.h"



void ConnectionManager::setGUICore(GUICore *gui) {
	guiCore = gui;
}

ImVec2 ConnectionManager::worldToScreen(const ImVec2& worldPos) const {
    return ImVec2(
        worldPos.x * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.x,
        worldPos.y * guiCore->viewportManager.zoomAmount + guiCore->viewportManager.viewOffset.y
    );
}

ImVec2 ConnectionManager::screenToWorld(const ImVec2& screenPos) const {
    return ImVec2(
        (screenPos.x - guiCore->viewportManager.viewOffset.x) / guiCore->viewportManager.zoomAmount,
        (screenPos.y - guiCore->viewportManager.viewOffset.y) / guiCore->viewportManager.zoomAmount
    );
}

void ConnectionManager::drawSingleConnection(Connection& conn, const ImVec2& mousePos, const ImVec2& mousePosWorld, ImDrawList* drawList) {
    if (!conn.sourceBlock || !conn.targetBlock)
        return;
    
    // Oblicz pozycje początku i końca
    ImVec2 source_pos = worldToScreen(conn.sourceBlock->position);
    ImVec2 source_size = ImVec2(
        conn.sourceBlock->size.x * guiCore->viewportManager.zoomAmount,
        conn.sourceBlock->size.y * guiCore->viewportManager.zoomAmount
    );
    
    ImVec2 target_pos = worldToScreen(conn.targetBlock->position);
    ImVec2 target_size = ImVec2(
        conn.targetBlock->size.x * guiCore->viewportManager.zoomAmount,
        conn.targetBlock->size.y * guiCore->viewportManager.zoomAmount
    );
    
    ImVec2 p1 = ImVec2(source_pos.x + source_size.x, source_pos.y + source_size.y * 0.5f);
    ImVec2 p2 = ImVec2(target_pos.x, target_pos.y + target_size.y * 0.5f);
    
    // rysuj węzły kontrolne
    const float nodeRadius = 6.0f;
    bool anyNodeHovered = false;
    
    for (size_t i = 0; i < conn.controlNodes.size(); ++i) {
        ImVec2 nodeScreenPos = worldToScreen(conn.controlNodes[i]);
        bool nodeHovered = LengthSqr(nodeScreenPos, mousePos) < (nodeRadius * 3.0f) * (nodeRadius * 3.0f);
        
        if (nodeHovered)
            anyNodeHovered = true;
        
        ImU32 nodeColor = nodeHovered ? IM_COL32(255, 150, 0, 255) : IM_COL32(200, 200, 200, 200);
        float visualRadius = nodeRadius * guiCore->viewportManager.zoomAmount;
        
        drawList->AddCircleFilled(nodeScreenPos, visualRadius, nodeColor);
        drawList->AddCircle(nodeScreenPos, visualRadius, IM_COL32(50, 50, 50, 255), 12, 2.0f);
        
        if (nodeHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !draggingNode)
            draggingNode = DraggingNode{&conn, (int)i};
        
        if (nodeHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            conn.controlNodes.erase(conn.controlNodes.begin() + i);
            return;
        }
    }
    
    // sprawdź hover na krzywej
    const float detectRadius = 10.0f * guiCore->viewportManager.zoomAmount;
    bool curveHovered = false;
    ImVec2 c1 = ImVec2(p1.x + 50 * guiCore->viewportManager.zoomAmount, p1.y);
    ImVec2 c2 = ImVec2(p2.x - 50 * guiCore->viewportManager.zoomAmount, p2.y);
    
    for (int step = 0; step <= 30; ++step) {
        float t = step / 30.0f;
        ImVec2 pt = BezierCubicCalc(p1, c1, c2, p2, t);
        if (LengthSqr(pt, mousePos) < detectRadius * detectRadius) {
            curveHovered = true;
            break;
        }
    }
    
    // rysuj krzywą
    ImU32 curveColor = (curveHovered && !anyNodeHovered) ? IM_COL32(255, 100, 100, 255) : IM_COL32(255, 255, 0, 255);
    float thickness = ((curveHovered && !anyNodeHovered) ? 5.0f : 3.0f) * guiCore->viewportManager.zoomAmount;

    // lista wszystkich punktów po kolei: p1 -> controlNodes -> p2
    std::vector<ImVec2> points;
    points.push_back(p1);
    for (auto& node : conn.controlNodes)
        points.push_back(worldToScreen(node));
    points.push_back(p2);

    // narysuj linię między każdym sąsiednim punktem
    for (size_t i = 0; i + 1 < points.size(); ++i)
        drawList->AddBezierCubic(
            points[i],
            ImVec2(points[i].x + 50 * guiCore->viewportManager.zoomAmount, points[i].y),
            ImVec2(points[i+1].x - 50 * guiCore->viewportManager.zoomAmount, points[i+1].y),
            points[i+1],
            curveColor,
            thickness
        );

    
    // double-click dodaje węzeł
    if (curveHovered && !anyNodeHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        conn.controlNodes.push_back(mousePosWorld);
    }
    
    // PPM usuwa połączenie
    if (curveHovered && !anyNodeHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        auto& connections = guiCore->model.getConnections();
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                [&](const Connection& c) {
                    return c.sourceBlock == conn.sourceBlock && c.targetBlock == conn.targetBlock;
                }),
            connections.end()
        );
        
        if (conn.sourceBlock) {
            auto& sourceConns = conn.sourceBlock->connections;
            sourceConns.erase(
                std::remove(sourceConns.begin(), sourceConns.end(), conn.targetBlock->id),
                sourceConns.end()
            );
        }
    }
}

void ConnectionManager::handleNodeDragging(const ImVec2& mousePosWorld) {
    if (draggingNode && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (draggingNode->connection && draggingNode->nodeIndex < draggingNode->connection->controlNodes.size())
            draggingNode->connection->controlNodes[draggingNode->nodeIndex] = mousePosWorld;
    }
    
    if (draggingNode && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
        draggingNode.reset();
}

void ConnectionManager::handleConnectionCreation(const ImVec2& mousePos, ImDrawList* drawList) {
    // rysuj dynamiczną linię podczas tworzenia połączenia
    if (draggingFrom) {
        auto it = std::find_if(guiCore->model.getBlocks().begin(),
                              guiCore->model.getBlocks().end(),
                              [&](auto& b) { return b->id == *draggingFrom; });

        if (it != guiCore->model.getBlocks().end()) {
            ImVec2 source_pos = worldToScreen((*it)->position);
            ImVec2 source_size = ImVec2(
                (*it)->size.x * guiCore->viewportManager.zoomAmount,
                (*it)->size.y * guiCore->viewportManager.zoomAmount
            );

            ImVec2 p1 = ImVec2(source_pos.x + source_size.x, source_pos.y + source_size.y * 0.5f);
            ImVec2 p2 = mousePos;

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

    // zakończenie przeciągania (puszczenie LPM)
    if (draggingFrom && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        for (auto& box : guiCore->model.getBlocks()) {
            ImVec2 boxMin = worldToScreen(box->position);
            ImVec2 boxSize = ImVec2(
                box->size.x * guiCore->viewportManager.zoomAmount,
                box->size.y * guiCore->viewportManager.zoomAmount
            );
            ImVec2 boxMax = ImVec2(boxMin.x + boxSize.x, boxMin.y + boxSize.y);

            if (mousePos.x >= boxMin.x && mousePos.x <= boxMax.x && mousePos.y >= boxMin.y && mousePos.y <= boxMax.y) {
                if (box->id != *draggingFrom && box->getNumInputs() > 0) {
                    // dodaj do lokalnej listy połączeń bloku
                    box->connections.push_back(*draggingFrom);

                    // dodanie do globalnej listy połączeń w modelu
                    auto source = guiCore->model.findBlockById(*draggingFrom);
                    if (source) {
                        Connection newConn;
                        newConn.sourceBlock = source;
                        newConn.targetBlock = box;
                        guiCore->model.getConnections().push_back(newConn);
                    }
                }

                break;
            }
        }

        // resetuj stan
        draggingFrom.reset();
    }
}


void ConnectionManager::drawConnections() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 mousePosWorld = screenToWorld(mousePos);
    
    // rysuj wszystkie połączenia
    for (auto& conn : guiCore->model.getConnections()) {
        drawSingleConnection(conn, mousePos, mousePosWorld, drawList);
    }
    
    // obsługa przeciągania węzłów
    handleNodeDragging(mousePosWorld);
    
    // obsługa tworzenia nowych połączeń
    handleConnectionCreation(mousePos, drawList);
}