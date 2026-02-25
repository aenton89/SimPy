//
// Created by tajbe on 12.11.2025.
//
#pragma once

#include "../BasicManager.h"
#include "../../../../ui/UIStyles.h"
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
        float pathDetectionRadius = 10.0f;
        float bezierControlOffset = 50.0f;
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
    bool isPointNearCurvePath(const ImVec2& point, const std::vector<ImVec2>& pathPoints, float threshold) const;
    [[nodiscard]]
    bool isPointNearOrthogonalPath(const ImVec2& point, const std::vector<ImVec2>& pathPoints, float threshold) const;

    // rysowanie
    void drawConnection(Connection& conn, ImDrawList* drawList);
    void drawConnectionPath(const std::vector<ImVec2>& points, bool hovered, ImDrawList* drawList) const;
    void drawControlNodes(Connection& conn, const ImVec2& mousePos, ImDrawList* drawList);
    void drawDraftConnection(ImDrawList* drawList) const;

    // obsługa interakcji
    void handleNodeDragging(const ImVec2& mousePosWorld);
    void handleConnectionCreation(const ImVec2& mousePos);
    void handleConnectionDeletion(const Connection& conn, bool pathHovered) const;

    static void handleNodeAddition(Connection& conn, bool pathHovered, const ImVec2& mousePosWorld);

public:
    bool useOrthogonalLines = false;

    ConnectionManager() = default;

    // główna metoda zawiera i rysowanie i obsługę połączeń
    void drawConnections();

    // API do rozpoczęcia tworzenia połączenia (wywoływane w BlocksManager)
    void startConnectionDraft(int blockId, int portIndex);
    // TODO: czy ta metoda jest w ogóle potrzebna?
    void cancelConnectionDraft();

    // pod usuwanie wielu bloczków przez DELETE
    void removeConnectionsForBlocks(const std::set<int>& blockIds) const;

    [[nodiscard]]
    bool isDraftingConnection() const;
    [[nodiscard]]
    bool isDraggingNode() const;

    // do serializacji
    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<BasicManager>(this));
        ar(CEREAL_NVP(useOrthogonalLines));

        if constexpr (Archive::is_loading::value) {
            // reset stanu runtime
            currentDraft.reset();
            draggingNode.reset();
        }
    }
};