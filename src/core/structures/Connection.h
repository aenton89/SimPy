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
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
// dla serializacji - pod polimorfizm
#include <cereal/types/polymorphic.hpp>
// dla serializacji pointer'ów
#include <cereal/types/memory.hpp>
#include "../../data/math/solvers/SolverManager.h"
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
	}
};