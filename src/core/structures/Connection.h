//
// Created by tajbe on 12.11.2025.
//
#pragma once

#include "BasicBlock.h"
// #include <imgui.h>
// #include <vector>
// #include <memory>
// #include <cereal/types/array.hpp>



/*
 * struktura reprezentująca pojedyncze połączenie między blokami
 * jedno połączenie = jeden port wyjściowy -> jeden port wejściowy
 */
struct Connection {
    // bloki i porty
    std::shared_ptr<Block> sourceBlock;
    int sourcePort = 0;
    std::shared_ptr<Block> targetBlock;
    int targetPort = 0;

    // węzły kontrolne krzywej
    std::vector<ImVec2> controlNodes;

    Connection() = default;
    Connection(std::shared_ptr<Block> src, int srcPort, std::shared_ptr<Block> tgt, int tgtPort);

    [[nodiscard]]
    bool isValid() const;

    // do sprawdzania duplikatów
    [[nodiscard]]
    bool matches(const std::shared_ptr<Block>& src, int srcP, const std::shared_ptr<Block>& tgt, int tgtP) const;

    // dla serializacji
    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(sourceBlock),
           CEREAL_NVP(sourcePort),
           CEREAL_NVP(targetBlock),
           CEREAL_NVP(targetPort));

        if constexpr (Archive::is_saving::value) {
            std::vector<std::array<float, 2>> temp;
            temp.reserve(controlNodes.size());
            for (const auto& v : controlNodes)
                temp.push_back({v.x, v.y});
            ar(cereal::make_nvp("controlNodes", temp));
        } else {
            std::vector<std::array<float, 2>> temp;
            ar(cereal::make_nvp("controlNodes", temp));
            controlNodes.clear();
            controlNodes.reserve(temp.size());
            for (const auto& t : temp)
                controlNodes.emplace_back(t[0], t[1]);
        }
    }
};