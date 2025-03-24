//
// Created by tajbe on 24.03.2025.
//

#include "structures.h"



// implementacja metod klasy Block
Block::Block(const std::string& blockName, int numInputs, int numOutputs)
    : name(blockName) {
    inputValues.resize(numInputs, 0.0);
    outputValues.resize(numOutputs, 0.0);
}

const std::string& Block::getName() const {
    return name;
}

void Block::setInput(int port, double value) {
    if (port >= 0 && port < inputValues.size()) {
        inputValues[port] = value;
    }
}

double Block::getOutput(int port) const {
    if (port >= 0 && port < outputValues.size()) {
        return outputValues[port];
    }
    return 0.0;
}

int Block::getNumInputs() const {
    return inputValues.size();
}

int Block::getNumOutputs() const {
    return outputValues.size();
}




// implementacja struktury Connection
Connection::Connection(Block* src, int srcPort, Block* tgt, int tgtPort)
    : sourceBlock(src), sourcePort(srcPort),
      targetBlock(tgt), targetPort(tgtPort) {}




// implementacja metod klasy Model
bool Model::hasCycleDFS(int v) {
    visited[v] = true;
    inStack[v] = true;

    for (size_t i = 0; i < blocks.size(); i++) {
        if (adjacencyMatrix[v][i]) {
            if (!visited[i] && hasCycleDFS(i)) {
                return true;
            } else if (inStack[i]) {
                // znaleziono cykl
                return true;
            }
        }
    }

    // usunięcie z bieżącego stosu DFS
    inStack[v] = false;
    return false;
}

void Model::updateAdjacencyMatrix() {
    size_t n = blocks.size();
    adjacencyMatrix.resize(n, std::vector<bool>(n, false));

    for (const auto& conn : connections) {
        int srcIdx = getBlockIndex(conn.sourceBlock);
        int tgtIdx = getBlockIndex(conn.targetBlock);

        if (srcIdx >= 0 && tgtIdx >= 0) {
            adjacencyMatrix[srcIdx][tgtIdx] = true;
        }
    }
}

int Model::getBlockIndex(Block* block) const {
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i].get() == block) {
            return i;
        }
    }
    return -1;
}

bool Model::connect(Block* source, int sourcePort, Block* target, int targetPort) {
    if (!source || !target) return false;
    if (sourcePort < 0 || sourcePort >= source->getNumOutputs()) return false;
    if (targetPort < 0 || targetPort >= target->getNumInputs()) return false;

    connections.emplace_back(source, sourcePort, target, targetPort);

    // aktualizacja macierzy i sprawdzenie cykli jeśli są problematyczne
    updateAdjacencyMatrix();

    return true;
}

bool Model::hasCycles() {
    visited.resize(blocks.size(), false);
    inStack.resize(blocks.size(), false);

    for (size_t i = 0; i < blocks.size(); i++) {
        if (!visited[i]) {
            if (hasCycleDFS(i)) {
                return true;
            }
        }
    }

    return false;
}

void Model::simulate() {
    // jeśli mamy cykle, W TEORII potrzebna jest specjalna obsługa (np. iteracyjne rozwiązanie) -> ALE narazie ignorujemy problem
    // bo i tak w naszym przypadku cykle są na sprzężeniach zwrotnych, a w nich ustaliliśmy, że wstawiamy 0.0
    bool hasCyclesInModel = hasCycles();

    // zbieranie wejść dla każdego bloku
    for (const auto& conn : connections) {
        double value = conn.sourceBlock->getOutput(conn.sourcePort);
        conn.targetBlock->setInput(conn.targetPort, value);
    }

    // przetwarzanie bloków
    for (auto& block : blocks) {
        block->process();
    }

    // jeśli mamy cykle, możemy potrzebować dodatkowych iteracji do zbieżności
    // ALE, ignorujemy, tho zostawiam komentarz i bloczek warunkowy na przyszłość
    if (hasCyclesInModel) {
        // tutaj np. implementacja iteracyjnego rozwiązania dla cykli
        // np. powtórz symulację kilka razy, aż do osiągnięcia zbieżności
    }
}

void Model::simulateMultipleSteps(int steps) {
    for (int i = 0; i < steps; i++) {
        simulate();
    }
}

const std::vector<std::unique_ptr<Block>>& Model::getBlocks() const {
    return blocks;
}

const std::vector<Connection>& Model::getConnections() const {
    return connections;
}