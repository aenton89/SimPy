//
// Created by tajbe on 12.11.2025.
//
#pragma once

#include "../BasicManager.h"
#include "../../../core/structures/Connection.h"
// #include <imgui.h>



/*
 * informacja o połączeniu dla ConnectionManager'a
 * używana podczas drag & drop tworzenia połączenia
 */
struct ConnectionDraft {
    int sourceBlockId = -1;
    int sourcePort = 0;

    [[nodiscard]]
    bool isActive() const {
        return sourceBlockId >= 0;
    }

    void reset() {
        sourceBlockId = -1; sourcePort = 0;
    }
};

/*
 * informacja o przeciąganym węźle kontrolnym
 */
struct DraggingNodeInfo {
    Connection* connection = nullptr;
    size_t nodeIndex = 0;

    [[nodiscard]]
    bool isActive() const {
        return connection != nullptr;
    }

    void reset() {
        connection = nullptr; nodeIndex = 0;
    }
};



/*
 * manages drawing connection between blocksć
 */
class ConnectionManager : public BasicManager {
private:
    // obecny stan tymczasowy - nie jest serializowane
    // trwające tworzenie połączenia
    ConnectionDraft currentDraft;
    // przeciągany węzeł kontrolny
    DraggingNodeInfo draggingNode;

    // TODO: parametry przy rysowaniu - gdzieś je przenieść? do UI?
    struct DrawSettings {
        float normalThickness = 3.0f;
        float hoveredThickness = 5.0f;
        float nodeRadius = 6.0f;
        float curveDetectionRadius = 10.0f;
        float bezierControlOffset = 50.0f;

        ImU32 normalColor = IM_COL32(255, 255, 0, 255);
        ImU32 hoveredColor = IM_COL32(255, 100, 100, 255);
        ImU32 draftColor = IM_COL32(255, 255, 0, 100);
        ImU32 nodeNormalColor = IM_COL32(200, 200, 200, 200);
        ImU32 nodeHoveredColor = IM_COL32(255, 150, 0, 255);
    } drawSettings;

    // helpery do konwersji współrzędnych
    [[nodiscard]]
    ImVec2 worldToScreen(const ImVec2& worldPos) const;
    [[nodiscard]]
    ImVec2 screenToWorld(const ImVec2& screenPos) const;

    [[nodiscard]]
    ImVec2 getBlockOutputPos(const Block& block) const;
    [[nodiscard]]
    ImVec2 getBlockInputPos(const Block& block) const;
    [[nodiscard]]
    bool isPointNearCurve(const ImVec2& point, const std::vector<ImVec2>& curvePoints, float threshold) const;

    // rysowanie
    void drawConnection(Connection& conn, ImDrawList* drawList);
    void drawConnectionCurve(const std::vector<ImVec2>& points, bool hovered, ImDrawList* drawList);
    void drawControlNodes(Connection& conn, const ImVec2& mousePos, ImDrawList* drawList);
    void drawDraftConnection(ImDrawList* drawList);

    // obsługa interakcji
    void handleNodeDragging(const ImVec2& mousePosWorld);
    void handleConnectionCreation(const ImVec2& mousePos);
    void handleConnectionDeletion(Connection& conn, bool curveHovered);
    void handleNodeAddition(Connection& conn, bool curveHovered, const ImVec2& mousePosWorld);

public:
    ConnectionManager() = default;

    // główna metoda zawiera i rysowanie i obsługę połączeń
    void drawConnections();

    // API do rozpoczęcia tworzenia połączenia (wywoływane w BlocksManager)
    void startConnectionDraft(int blockId, int portIndex);
    // TODO: czy ta metoda jest w ogóle potrzebna? 0
    void cancelConnectionDraft();

    [[nodiscard]]
    bool isDraftingConnection() const;

    // do serializacji
    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<BasicManager>(this));

        if constexpr (Archive::is_loading::value) {
            // reset stanu runtime
            currentDraft.reset();
            draggingNode.reset();
        }
    }
};