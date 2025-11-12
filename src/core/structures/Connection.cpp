//
// Created by tajbe on 12.11.2025.
//
#include "Connection.h"



Connection::Connection(std::shared_ptr<Block> src, int srcPort, std::shared_ptr<Block> tgt, int tgtPort)
	: sourceBlock(src), sourcePort(srcPort),
	  targetBlock(tgt), targetPort(tgtPort) {}