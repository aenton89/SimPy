//
// Created by tajbe on 12.11.2025.
//
#include "Connection.h"
#include <utility>



Connection::Connection(std::shared_ptr<Block> src, int srcPort, std::shared_ptr<Block> tgt, int tgtPort)
	: sourceBlock(std::move(src)), sourcePort(srcPort), targetBlock(std::move(tgt)), targetPort(tgtPort) {}

bool Connection::isValid() const {
	return sourceBlock && targetBlock &&
		   sourcePort >= 0 && sourcePort < sourceBlock->getNumOutputs() &&
		   targetPort >= 0 && targetPort < targetBlock->getNumInputs();
}

bool Connection::matches(const std::shared_ptr<Block>& src, int srcP, const std::shared_ptr<Block>& tgt, int tgtP) const {
	return sourceBlock == src && sourcePort == srcP && targetBlock == tgt && targetPort == tgtP;
}