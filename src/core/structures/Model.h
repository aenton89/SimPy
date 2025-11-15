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
#include "Connection.h"



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
    int getBlockIndex(std::shared_ptr<Block> block) const;

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

    // dodanie połączenia między blokami do modelu
    bool connect(std::shared_ptr<Block> source, int sourcePort, std::shared_ptr<Block> target, int targetPort);
    void disconnectAll();

    // sprawdzenie czy model zawiera cykle
    bool hasCycles();

    // symulacja, pojedyńczy krok
    void simulate();
    // symulacja, wiele kroków
    void simulateMultipleSteps(int steps);

    // czyszczenie na koniec symulacji
    void cleanupBefore();
    void cleanupAfter();

    // gettery listy bloków i połączeń - przeciążone na const i nie-const
    std::vector<std::shared_ptr<Block>>& getBlocks();
    std::vector<Connection>& getConnections();
    const std::vector<std::shared_ptr<Block>>& getBlocks() const;
    const std::vector<Connection>& getConnections() const;

    // TODO: łączenie w całość
    void makeConnections();
    // metoda pomocnicza do usuwania polaczen, ma je robic od nowa; TODO:(to niby gdzie jest xd??)

    // inplentacja solvera dla symulacji
    void setSolver(std::shared_ptr<Solver> solver);
    void cleanSolver();

    // pod dodanie węzłów do krzywych
    Connection* findConnection(std::shared_ptr<Block> source, std::shared_ptr<Block> target);
    std::shared_ptr<Block> findBlockById(int id) const;

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