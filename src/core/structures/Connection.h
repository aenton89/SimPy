//
// Created by tajbe on 12.11.2025.
//
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cmath>
#include <imgui.h>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/complex.hpp>
#include "BasicBlock.h"



/* struktura reprezentująca połączenie między blokami
- pomaga z procesem łączenia bloczków
- oraz flow danych między nimi
- sprzężenia zwrotne itd. też są tu obsługiwane
 */
struct Connection {
	std::shared_ptr<Block> sourceBlock;
	int sourcePort;
	std::shared_ptr<Block> targetBlock;
	int targetPort;
	// dodatkowe węzły kontrolne na krzywej (punkty do manipulacji)
	std::vector<ImVec2> controlNodes;

	// konstruktor
	Connection() = default;
	Connection(std::shared_ptr<Block> src, int srcPort, std::shared_ptr<Block> tgt, int tgtPort);

	// dla serializacji
	template<class Archive>
	void serialize(Archive& ar) {
		ar(CEREAL_NVP(sourceBlock),
		   CEREAL_NVP(sourcePort),
		   CEREAL_NVP(targetBlock),
		   CEREAL_NVP(targetPort));

		// jeszcze zapis controlNodes
		if constexpr (Archive::is_saving::value) {
			std::vector<std::array<float, 2>> temp;
			temp.reserve(controlNodes.size());
			for (auto& v : controlNodes)
				temp.push_back({v.x, v.y});
			ar(cereal::make_nvp("controlNodes", temp));
		} else {
			std::vector<std::array<float, 2>> temp;
			ar(cereal::make_nvp("controlNodes", temp));
			controlNodes.clear();
			controlNodes.reserve(temp.size());
			for (auto& t : temp)
				controlNodes.emplace_back(t[0], t[1]);
		}
	}
};