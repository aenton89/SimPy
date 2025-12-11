//
// Created by tajbe on 12.11.2025.
//
#include "Model.h"



double Model::timeStep = 0.1;
double Model::simTime = 10.0;



bool Model::hasCycleDFS(int v) {
    visited[v] = true;
    inStack[v] = true;

    for (size_t i = 0; i < blocks.size(); i++) {
        if (adjacencyMatrix[v][i]) {
            if ((!visited[i] && hasCycleDFS(i)) || inStack[i])
                return true;
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

        if (srcIdx >= 0 && tgtIdx >= 0)
            adjacencyMatrix[srcIdx][tgtIdx] = true;
    }
}

int Model::getBlockIndex(const std::shared_ptr<Block> &block) const {
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i] == block)
            return static_cast<int>(i);
    }
    return -1;
}

/* TODO:
 * ogólnie to tu narazie jest robienie tych połączeń troche w losowej kolejności xd
 * tym sie zająć kiedyś coś
 * trzeba to poprawić i w GuiClass przy samym dodawaniu połączeń
 * (może zamiast vector w Blocks zrobić pair, gdzie też jest na który Port)
 * i tutaj, w miejścu poniżej oznaczonym jako TODO
 * (bo tam są podane sourcePort i targetPort)
 */

void Model::disconnectAll() {
    connections.clear();
}

bool Model::hasCycles() {
    visited.resize(blocks.size(), false);
    inStack.resize(blocks.size(), false);

    for (size_t i = 0; i < blocks.size(); i++) {
        if (!visited[i]) {
            if (hasCycleDFS(i))
                return true;
        }
    }

    return false;
}

void Model::simulate() const {
    if (!connections.empty()) {
        // jeśli mamy cykle, W TEORII potrzebna jest specjalna obsługa (np. iteracyjne rozwiązanie) -> ALE narazie ignorujemy problem
        // bo i tak w naszym przypadku cykle są na sprzężeniach zwrotnych, a w nich ustaliliśmy, że wstawiamy 0.0
        // bool hasCyclesInModel = hasCycles();

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
        // if (hasCyclesInModel) {
            // tutaj np. implementacja iteracyjnego rozwiązania dla cykli
            // np. powtórz symulację kilka razy, aż do osiągnięcia zbieżności
        //}
    }
}

void Model::simulateMultipleSteps(int steps) const {
    for (int i = 0; i < steps; i++) {
        simulate();
    }
}

std::vector<std::shared_ptr<Block>>& Model::getBlocks() {
    return blocks;
}

std::vector<Connection>& Model::getConnections() {
    return connections;
}

const std::vector<std::shared_ptr<Block>>& Model::getBlocks() const {
    return blocks;
}

const std::vector<Connection>& Model::getConnections() const {
    return connections;
}

void Model::removeBlock(int removeId) {
    std::erase_if(blocks, [removeId](const std::shared_ptr<Block>& block) {
                    return block->getId() == removeId;
                });
}

void Model::cleanupAfter() const {
    for (auto& block : blocks) {
        // resetujemy stan każdego bloku
        block->resetAfter();
    }
}

void Model::cleanupBefore() const {
    for (auto& block : blocks) {
        // resetujemy stan każdego bloku
        block->resetBefore();
    }
}

// metody dla solvera
void Model::setSolver(std::shared_ptr<Solver> s) {
    solver = std::move(s);
}

void Model::cleanSolver() {
    solver.reset();
}

std::shared_ptr<Block> Model::findBlockById(int id) const {
    for (auto& block : blocks) {
        if (block->id == id)
            return block;
    }
    return nullptr;
}

bool Model::addConnection(std::shared_ptr<Block>& source, int sourcePort, const std::shared_ptr<Block>& target, int targetPort) {
    // podstawowa walidacja
    if (!source || !target) {
        std::cerr << "Null blocks in connection\n";
        return false;
    }
    if (source == target) {
        std::cerr << "Cannot connect block to itself\n";
        return false;
    }
    if (sourcePort < 0 || sourcePort >= source->getNumOutputs()) {
        std::cerr << "Invalid source port: " << sourcePort << "\n";
        return false;
    }
    if (targetPort < 0 || targetPort >= target->getNumInputs()) {
        std::cerr << "Invalid target port: " << targetPort << "\n";
        return false;
    }

    // czy to połączenie już istnieje
    if (hasConnection(source, sourcePort, target, targetPort)) {
        std::cerr << "Connection already exists\n";
        return false;
    }

    // czy port wejściowy nie jest już zajęty
    if (isInputPortUsed(target, targetPort)) {
        std::cerr << "Target port " << targetPort << " already connected\n";
        return false;
    }

    // dodanie połączenia + aktualizacja liczników portów
    connections.emplace_back(source, sourcePort, target, targetPort);
    source->setCurrentNumOutputs(source->getCurrentNumOutputs() + 1);
    target->setCurrentNumInputs(target->getCurrentNumInputs() + 1);

    // TODO: sprawdzanie cykli - idk co z tym wszystkim, chyba program działa, trzeba kiedyś sprawdzić edge case'y
    // updateAdjacencyMatrix();
    // if (hasCycles())
    //     std::cerr << "Warning: Connection creates a cycle\n";

    return true;
}

bool Model::removeConnection(const std::shared_ptr<Block>& source, int sourcePort, const std::shared_ptr<Block>& target, int targetPort) {
    auto it = std::ranges::find_if(connections, [&](const Connection& c) {
            return c.matches(source, sourcePort, target, targetPort);
        });

    if (it != connections.end()) {
        // TODO: idk czy tego nie usunąć jak zmienimy model portów na możliwość wybrania, który port
        source->setCurrentNumOutputs(source->getCurrentNumOutputs() - 1);
        target->setCurrentNumInputs(target->getCurrentNumInputs() - 1);
        connections.erase(it);
        // TODO: sprawdzanie cykli
        // updateAdjacencyMatrix();
        return true;
    }

    return false;
}

void Model::removeAllConnectionsForBlock(const std::shared_ptr<Block>& block) {
    // zlicz ile połączeń usuniemy dla każdego bloku
    std::unordered_map<std::shared_ptr<Block>, int> outputsToRemove;
    std::unordered_map<std::shared_ptr<Block>, int> inputsToRemove;

    for (const auto& conn : connections) {
        if (conn.sourceBlock == block || conn.targetBlock == block) {
            // jeśli usuwany blok jest source'm
            if (conn.sourceBlock == block) {
                outputsToRemove[conn.sourceBlock]++;
                inputsToRemove[conn.targetBlock]++;
            }
            // jeśli usuwany blok jest target'em
            else if (conn.targetBlock == block) {
                outputsToRemove[conn.sourceBlock]++;
                inputsToRemove[conn.targetBlock]++;
            }
        }
    }

    // zaktualizuj liczniki przed usunięciem
    for (const auto& [blk, count] : outputsToRemove) {
        int newCount = blk->getCurrentNumOutputs() - count;
        // nie dopuść do wartości ujemnych
        blk->setCurrentNumOutputs(std::max(0, newCount));
    }

    for (const auto& [blk, count] : inputsToRemove) {
        int newCount = blk->getCurrentNumInputs() - count;
        // nie dopuść do wartości ujemnych
        blk->setCurrentNumInputs(std::max(0, newCount));
    }

    // TODO: wystarczy jak poniżej, jeśli zmienimy model portów na możliwość wybrania, który port:
    // usuń połączenia
    std::erase_if(connections, [&](const Connection& c) {
        return c.sourceBlock == block || c.targetBlock == block;
    });

    // TODO: sprawdzanie cykli
    // updateAdjacencyMatrix();
}

bool Model::hasConnection(const std::shared_ptr<Block>& source, int sourcePort, const std::shared_ptr<Block>& target, int targetPort) const {
    return std::ranges::any_of(connections, [&](const Connection& c) {
            return c.matches(source, sourcePort, target, targetPort);
        });
}

bool Model::isInputPortUsed(const std::shared_ptr<Block>& block, int port) const {
    return std::ranges::any_of(connections, [&](const Connection& c) {
            return c.targetBlock == block && c.targetPort == port;
        });
}

Connection* Model::findAnyConnectionBetween(const std::shared_ptr<Block>& source, const std::shared_ptr<Block>& target) {
    auto it = std::ranges::find_if(connections, [&](const Connection& c) {
            return c.sourceBlock == source && c.targetBlock == target;
        });

    return (it != connections.end()) ? &(*it) : nullptr;
}

Connection* Model::findConnection(const std::shared_ptr<Block>& source, int sourcePort, const std::shared_ptr<Block>& target, int targetPort) {
    auto it = std::ranges::find_if(connections, [&](const Connection& c) {
            return c.matches(source, sourcePort, target, targetPort);
        });

    return (it != connections.end()) ? &(*it) : nullptr;
}

std::vector<Connection*> Model::getOutputConnectionsFor(const std::shared_ptr<Block>& block) {
    std::vector<Connection*> result;
    for (auto& conn : connections) {
        if (conn.sourceBlock == block)
            result.push_back(&conn);
    }

    return result;
}

std::vector<Connection*> Model::getInputConnectionsFor(const std::shared_ptr<Block>& block) {
    std::vector<Connection*> result;
    for (auto& conn : connections) {
        if (conn.targetBlock == block)
            result.push_back(&conn);
    }

    return result;
}