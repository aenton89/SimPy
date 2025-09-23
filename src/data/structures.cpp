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
    if (port >= 0 && port < outputValues.size())
        return outputValues[port];
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
        if (ImGui::Button(">", ImVec2(20.0, 20.0)))
            ImGui::OpenPopup("MoreOptionsPopup");

        // samo menu
        if (ImGui::BeginPopup("MoreOptionsPopup")) {
            drawMenu();

            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }
}

// TODO: tu sobie patryk nadpisz dla ikonek
void Block::drawIcon() {
    // cośtam cośtam
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// implementacja struktury Connection
Connection::Connection(std::shared_ptr<Block> src, int srcPort, std::shared_ptr<Block> tgt, int tgtPort)
    : sourceBlock(src), sourcePort(srcPort),
      targetBlock(tgt), targetPort(tgtPort) {}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// implementacja metod klasy Model
double Model::timeStep = 0.1;
double Model::simTime = 10.0;

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

int Model::getBlockIndex(std::shared_ptr<Block> block) const {
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i] == block) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool Model::connect(std::shared_ptr<Block> source, int sourcePort, std::shared_ptr<Block> target, int targetPort) {
    if (!source || !target)
        return false;
    if (sourcePort < 0 || sourcePort >= source->getNumOutputs())
        return false;
    if (targetPort < 0 || targetPort >= target->getNumInputs())
        return false;

    connections.emplace_back(source, sourcePort, target, targetPort);

    // aktualizacja macierzy i sprawdzenie cykli jeśli są problematyczne
    updateAdjacencyMatrix();

    return true;
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
    disconnectAll(); // bardzo ważne – usuwamy stare połączenia

    for (auto& boxPtr : blocks) {
        boxPtr->numConnected = 0; // reset licznika!
        for (auto connId : boxPtr->connections) {
            auto it = std::find_if(blocks.begin(), blocks.end(),
                [connId](const std::shared_ptr<Block>& b) {
                    return b->id == connId;
                });

            if (it != blocks.end()) {
                auto connectedBlock = *it;
                connect(connectedBlock, 0, boxPtr, boxPtr->numConnected);
                boxPtr->numConnected++;
                // std::cout << "Connected block " << boxPtr->id
                //           << " to block " << connectedBlock->id << std::endl;
            } else {
                std::cerr << "Warning: Block with ID " << connId << " not found.\n";
            }
        }
    }
}


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

void Model::simulate() {
   // std::cout<<"simulate() called"<<std::endl;
    if (connections.size() > 0) {
        //std::cout<<"connections.size() > 0"<<std::endl;
        // jeśli mamy cykle, W TEORII potrzebna jest specjalna obsługa (np. iteracyjne rozwiązanie) -> ALE narazie ignorujemy problem
        // bo i tak w naszym przypadku cykle są na sprzężeniach zwrotnych, a w nich ustaliliśmy, że wstawiamy 0.0
        //bool hasCyclesInModel = hasCycles();

        // zbieranie wejść dla każdego bloku
        for (const auto& conn : connections) {
            double value = conn.sourceBlock->getOutput(conn.sourcePort);
            conn.targetBlock->setInput(conn.targetPort, value);
        }

        // przetwarzanie bloków
        for (auto& block : blocks) {
            //std::cout<<"Processing block ID: " << block->getId() << std::endl;
            block->process();
        }

        // jeśli mamy cykle, możemy potrzebować dodatkowych iteracji do zbieżności
        // ALE, ignorujemy, tho zostawiam komentarz i bloczek warunkowy na przyszłość
        //if (hasCyclesInModel) {
            // tutaj np. implementacja iteracyjnego rozwiązania dla cykli
            // np. powtórz symulację kilka razy, aż do osiągnięcia zbieżności
        //}
    }
}

void Model::simulateMultipleSteps(int steps) {
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


void Model::addBlock(std::shared_ptr<Block> block) {
    blocks.push_back(std::move(block));
}

void Model::removeBlock(int removeId) {
    blocks.erase(std::remove_if(blocks.begin(), blocks.end(),
        [removeId](const std::shared_ptr<Block>& block) {
            return block->getId() == removeId;
        }), blocks.end());
}

void Model::cleanupAfter() {
    for (auto& block : blocks) {
        // resetujemy stan każdego bloku
        block->resetAfter();
    }
}

void Model::cleanupBefore() {
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


