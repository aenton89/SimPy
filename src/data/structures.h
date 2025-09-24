//
// Created by tajbe on 24.03.2025.
//

#ifndef STRUCTURES_H
#define STRUCTURES_H

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
#include "math/solvers/solverManager.h"


/* klasa z której dziedziczą wszystkie bloki, odpowiada za:
- procesowanie danych
- przechowywanie danych wejściowych i wyjściowych
- liczbe wejść i wyjść
- nazwe bloku
 */
class Block {
protected:
    int numInputs;
    int numOutputs;
    bool has_menu;
    // TODO: potem wrócić to do protected
    std::vector<double> inputValues;
    std::vector<double> outputValues;
public:
    // TODO: GUI - zmienne od GUI
    int id;
    ImVec2 position = ImVec2(100, 100);
    ImVec2 size = ImVec2(120, 120);
    bool open = true;
    std::vector<int> connections;
    int numConnected = 0;

    Block(int _id, int _numInputs, int _numOutputs, bool _has_menu = false);
    virtual ~Block() = default;

    // TODO: GUI - metoda do rysowania
    virtual void drawContent();
    // tu stricte odpowiada za to co będzie narysowane w menu
    virtual void drawMenu() {};
    void drawIcon();

    // metoda dla kopiowania bloczków przez CTRL+D
    virtual std::unique_ptr<Block> clone() const = 0;

    // metoda do przetwarzania danych wejściowych i generowania danych wyjściowych
    virtual void process() = 0;

    // gettery i settery
    // const std::string& getName() const;
    void setInput(int port, double value);
    double getOutput(int port) const;

    int getNumInputs() const;
    int getNumOutputs() const;

    // TODO: spoza GUI
    int getId() const;

    // resetowanie stanu bloku - jeśli jest potrzebne
    virtual void resetBefore() {};
    virtual void resetAfter() {};

    #ifdef UNIT_TESTS
    public:
        std::vector<double>& getInputValues() { return inputValues; }
    #endif

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(id),
           CEREAL_NVP(numInputs),
           CEREAL_NVP(numOutputs),
           CEREAL_NVP(has_menu),
           CEREAL_NVP(inputValues),
           CEREAL_NVP(outputValues),
           CEREAL_NVP(connections),
           CEREAL_NVP(numConnected),
           CEREAL_NVP(open));

        std::vector<float> pos = {position.x, position.y};
        std::vector<float> sz  = {size.x, size.y};
        ar(cereal::make_nvp("position", pos), cereal::make_nvp("size", sz));

        if constexpr (Archive::is_loading::value) {
            position = ImVec2(pos[0], pos[1]);
            size = ImVec2(sz[0], sz[1]);
        }
    }
};



/* klasa pomocnicza do klonowania bloczków
- używa CRTP czy coś (Curiously Recurring Template Pattern)
- dziedziczy konstruktor po Block
- (note to self):
- szablony muszą być definiowane w pliku .h, bo kompilator musi widzieć ich definicję przy instancjonowaniu
 */
template <typename Derived>
class BlockCloneable : public Block {
public:
    // dziedziczymy konstruktory z Block
    using Block::Block;

    // dla serializacji
    BlockCloneable() : Block(-1, 0, 0, false) {}

    std::unique_ptr<Block> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};


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
    // przeniesione z guiClass
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


#endif