//
// Created by tajbe on 12.11.2025.
//
#include "Connection.h"
#include <utility>



Connection::Connection(std::shared_ptr<Block> src, int srcPort, std::shared_ptr<Block> tgt, int tgtPort)
	: sourceBlock(std::move(src)), sourcePort(srcPort), targetBlock(std::move(tgt)), targetPort(tgtPort) {}