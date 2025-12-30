//
// Created by tajbe on 12.11.2025.
//
#pragma once

#include <algorithm>
#include "../../data/math/solvers/SolverManager.h"
#include "BasicBlock.h"
#include "Connection.h"
// #include <vector>
// #include <memory>



/* główna klasa modelu (grafu)
- łączy w sobie połączenia i bloczki
- no i też zawiera całą symulację
 */
class Model {
private:
    // bloki i połączenia jakie mamy w modelu
    std::vector<std::shared_ptr<Block>> blocks;
    std::vector<Connection> connections;

    // macierz sąsiedztwa do wykrywania cykli (które narazie są nam średnio potrzebne, o ile w ogóle kiedyś będą)
    std::vector<std::vector<bool>> adjacencyMatrix;

    // "flagi" pod algorytm DFS
    std::vector<bool> visited;
    std::vector<bool> inStack;

    // wykrywanie cykli za pomocą DFS (metoda pomocnicza do hasCycles)
    bool hasCycleDFS(int v);

    // aktualizacja macierzy sąsiedztwa
    void updateAdjacencyMatrix();

    // znalezienie indeksu bloku
    [[nodiscard]]
    int getBlockIndex(const std::shared_ptr<Block> &block) const;

    // solver dla modelu
    std::shared_ptr<Solver> solver;

public:
    // te dwie zmainne zeby po przez menu mozna bylo ustawiac czas symuliociu i dt
    static double timeStep;
    static double simTime;

    // przeniesione z GUICore
    int next_id = 0;



    // dodanie i usuwanie bloku do modelu
    template<typename BlockType, typename... Args>
    std::shared_ptr<Block> addBlock(Args&&... args) {
        auto block = std::make_shared<BlockType>(next_id++, std::forward<Args>(args)...);
        blocks.push_back(block);
        return block;
    }
    void removeBlock(int removeId);

    void disconnectAll();

    // sprawdzenie czy model zawiera cykle
    bool hasCycles();

    // symulacja, pojedyńczy krok
    void simulate() const;
    // symulacja, wiele kroków
    void simulateMultipleSteps(int steps) const;

    // czyszczenie na koniec symulacji
    void cleanupBefore() const;
    void cleanupAfter() const;

    // gettery listy bloków i połączeń - przeciążone na const i nie-const
    std::vector<std::shared_ptr<Block>>& getBlocks();
    std::vector<Connection>& getConnections();
    [[nodiscard]]
    const std::vector<std::shared_ptr<Block>>& getBlocks() const;
    [[nodiscard]]
    const std::vector<Connection>& getConnections() const;

    // inplentacja solvera dla symulacji
    void setSolver(std::shared_ptr<Solver> solver);
    void cleanSolver();

    // pod dodanie węzłów do krzywych
    [[nodiscard]]
    std::shared_ptr<Block> findBlockById(int id) const;

    // dodaje połączenie z walidacją czy jest ono możliwe
    bool addConnection(std::shared_ptr<Block>& source, int sourcePort, const std::shared_ptr<Block>& target, int targetPort);
    // usuwa połączenie między konkretnymi portami
    bool removeConnection(const std::shared_ptr<Block>& source, int sourcePort, const std::shared_ptr<Block>& target, int targetPort);
    // usuwa wszystkie połączenia związane z blokiem - potrzebne do usuwania bloku
    void removeAllConnectionsForBlock(const std::shared_ptr<Block>& block);
    // sprawdza czy połączenie już istnieje
    [[nodiscard]]
    bool hasConnection(const std::shared_ptr<Block>& source, int sourcePort, const std::shared_ptr<Block>& target, int targetPort) const;
    // sprawdza czy port wejściowy jest już używany
    [[nodiscard]]
    bool isInputPortUsed(const std::shared_ptr<Block>& block, int port) const;
    // znajduje połączenie między blokami (dowolne porty)
    Connection* findAnyConnectionBetween(const std::shared_ptr<Block>& source, const std::shared_ptr<Block>& target);
    // znajduje konkretne połączenie między portami
    Connection* findConnection(const std::shared_ptr<Block>& source, int sourcePort, const std::shared_ptr<Block>& target, int targetPort);
    // zwraca wszystkie połączenia wychodzące z bloku
    std::vector<Connection*> getOutputConnectionsFor(const std::shared_ptr<Block>& block);
    // zwraca wszystkie połączenia wchodzące do bloku
    std::vector<Connection*> getInputConnectionsFor(const std::shared_ptr<Block>& block);

    // dla serializacji
    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(blocks),
           CEREAL_NVP(connections),
           CEREAL_NVP(timeStep),
           CEREAL_NVP(simTime),
           CEREAL_NVP(next_id));

        // reinitialize solver after loading
        if constexpr (Archive::is_loading::value) {
            // reset adjacency matrix and other computed fields
            updateAdjacencyMatrix();
        }
    }
};