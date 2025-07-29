//
// Created by tajbe on 24.03.2025.
//

#include "structures.h"



// ----------------------------------------------------------------------------------------------------------------------------------------------
// implementacja metod klasy Block
Block::Block(int _id, int _numInputs, int _numOutputs, bool _has_menu)
    : id(_id), numInputs(_numInputs), numOutputs(_numOutputs), has_menu(_has_menu) {
    inputValues.resize(numInputs, 0.0);
    outputValues.resize(numOutputs, 0.0);
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
    return numInputs;
}

int Block::getNumOutputs() const {
    return numOutputs;
}

int Block::getId() const {
    return id;
}

void Block::drawContent() {
    if (has_menu) {
        // przycisk rozwijający menu
        if (ImGui::Button(">", ImVec2(20.0, 20.0))) {
            ImGui::OpenPopup("MoreOptionsPopup");
        }

        // samo menu
        if (ImGui::BeginPopup("MoreOptionsPopup")) {
            drawMenu();

            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// implementacja struktury Connection
Connection::Connection(Block* src, int srcPort, Block* tgt, int tgtPort)
    : sourceBlock(src), sourcePort(srcPort),
      targetBlock(tgt), targetPort(tgtPort) {}



// ----------------------------------------------------------------------------------------------------------------------------------------------
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
    if (connections.size() > 0) {
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
}

void Model::simulateMultipleSteps(int steps) {
    for (int i = 0; i < steps; i++) {
        simulate();
    }
}

std::vector<std::unique_ptr<Block>>& Model::getBlocks() {
    return blocks;
}

std::vector<Connection>& Model::getConnections() {
    return connections;
}

const std::vector<std::unique_ptr<Block>>& Model::getBlocks() const {
    return blocks;
}

const std::vector<Connection>& Model::getConnections() const {
    return connections;
}


/* TODO:
 * ogólnie to tu narazie jest robienie tych połączeń troche w losowej kolejności xd
 * tym sie zająć kiedyś coś
 * trzeba to poprawić i w GuiClass przy samym dodawaniu połączeń
 * (może zamiast vector w Blocks zrobić pair, gdzie też jest na który Port)
 * i tutaj, w miejścu poniżej oznaczonym jako TODO
 * (bo tam są podane sourcePort i targetPort)
 */
void Model::makeConnections() {
    for (auto& boxPtr : blocks) {
        for (auto connId : boxPtr->connections) {
            // szukamy tego ze zgadzającym się id
            auto it = std::find_if(blocks.begin(), blocks.end(),
                [connId](const std::unique_ptr<Block>& b) {
                    return b->id == connId;
                });

            // tu tworzymy Connection
            if (it != blocks.end()) {
                Block *connectedBlock = it->get();
                // TODO: o tym mówi todo wyżej
                connect(connectedBlock, 0, boxPtr.get(), boxPtr->numConnected);
                boxPtr->numConnected++;
                std::cout << "Connected block " << boxPtr->id << " to block " << connectedBlock->id << std::endl;
            }
            // blok o connId nie znaleziony, idk może potem jakaś lepsza obsługę wymyśleć
            else {
                std::cerr << "Warning: Block with ID " << connId << " not found for connections." << std::endl;
            }
        }
    }
}


void Model::addBlock(std::unique_ptr<Block> block) {
    blocks.push_back(std::move(block));
}

void Model::removeBlock(int removeId) {
    blocks.erase(std::remove_if(blocks.begin(), blocks.end(),
        [removeId](const std::unique_ptr<Block>& block) {
            return block->getId() == removeId;
        }), blocks.end());
}

void Model::cleanup() {
    for (auto& block : blocks) {
        block->reset();  // resetujemy stan każdego bloku
    }
}
