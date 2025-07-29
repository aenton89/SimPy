//
// Created by tajbe on 29.07.2025.
//

//#include "structuresSerialization.h"
//
//
//
//// ----- CONNECTION -----
//void to_json(json& j, const Connection& c) {
//    j = json{
//        {"source_id", c.sourceBlock ? c.sourceBlock->getId() : -1},
//        {"source_port", c.sourcePort},
//        {"target_id", c.targetBlock ? c.targetBlock->getId() : -1},
//        {"target_port", c.targetPort}
//    };
//}
//
//void from_json(const json& j, Connection& c) {
//    // UWAGA: Pełna rekonstrukcja wymaga mapy id → Block* i powinna być w Model::makeConnections()
//    c.sourceBlock = nullptr;
//    c.targetBlock = nullptr;
//    c.sourcePort = j.at("source_port").get<int>();
//    c.targetPort = j.at("target_port").get<int>();
//}
//
//// ----- BLOCK (tylko to_json, bo deserializacja jest po stronie klas dziedziczących) -----
//void to_json(json& j, const Block& b) {
//    j = json{
//        {"id", b.getId()},
//        {"position", {b.position.x, b.position.y}},
//        {"size", {b.size.x, b.size.y}},
//        {"open", b.open},
//        {"numInputs", b.getNumInputs()},
//        {"numOutputs", b.getNumOutputs()},
//        {"type", b.getTypeName()},  // to musi być dodane jako wirtualna metoda np. getTypeName()
//        {"inputs", b.inputValues},
//        {"outputs", b.outputValues}
//    };
//}
//
//// ----- MODEL -----
//void to_json(json& j, const Model& m) {
//    json blocksJson = json::array();
//    for (const auto& block : m.getBlocks()) {
//        blocksJson.push_back(*block);
//    }
//
//    json connectionsJson = json::array();
//    for (const auto& conn : m.getConnections()) {
//        connectionsJson.push_back(conn);
//    }
//
//    j = json{
//        {"blocks", blocksJson},
//        {"connections", connectionsJson}
//    };
//}
//
//void from_json(const json& j, Model& m) {
//    const auto& blocksJson = j.at("blocks");
//    const auto& connectionsJson = j.at("connections");
//
//    // Bloki tworzyć osobno, np. przez rejestr deserializacji (np. po "type")
//    for (const auto& b : blocksJson) {
//        // tutaj dać np. createBlockFromJson(b); potem m.addBlock(...)
//    }
//
//    for (const auto& c : connectionsJson) {
//        Connection conn = c.get<Connection>();
//        m.getConnections().push_back(conn);
//    }
//
//    // Po załadowaniu bloków i połączeń trzeba podmienić sourceBlock/targetBlock w Connection
//    m.makeConnections();
//}
