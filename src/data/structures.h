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


/* klasa z której dziedziczą wszystkie bloki, odpowiada za:
- procesowanie danych
- przechowywanie danych wejściowych i wyjściowych
- liczbe wejść i wyjść
- nazwe bloku
 */
class Block {
protected:
    std::string name;
    std::vector<double> inputValues;
    std::vector<double> outputValues;

public:
    Block(const std::string& blockName, int numInputs, int numOutputs);
    virtual ~Block() = default;

    // metoda do przetwarzania danych wejściowych i generowania danych wyjściowych
    virtual void process() = 0;

    // gettery i settery
    const std::string& getName() const;
    void setInput(int port, double value);
    double getOutput(int port) const;
    int getNumInputs() const;
    int getNumOutputs() const;
};


/* struktura reprezentująca połączenie między blokami
- pomaga z procesem łączenia bloczków
- oraz flow danych między nimi
- sprzężenia zwrotne itd. też są tu obsługiwane
 */
struct Connection {
    Block* sourceBlock;
    int sourcePort;
    Block* targetBlock;
    int targetPort;

    // konstruktor
    Connection(Block* src, int srcPort, Block* tgt, int tgtPort);
};


/* główna klasa modelu (grafu)
- łączy w sobie połączenia i bloczki
- no i też zawiera całą symulację
 */
class Model {
private:
    // bloki i połączenia jakie mamy w modelu
    std::vector<std::unique_ptr<Block>> blocks;
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
    int getBlockIndex(Block* block) const;

public:
    // dodanie bloku do modelu
    template<typename BlockType, typename... Args>
    BlockType* addBlock(Args&&... args) {
        auto block = std::make_unique<BlockType>(std::forward<Args>(args)...);
        BlockType* ptr = block.get();
        blocks.push_back(std::move(block));
        return ptr;
    }
    // dodanie połączenia między blokami do modelu
    bool connect(Block* source, int sourcePort, Block* target, int targetPort);

    // sprawdzenie czy model zawiera cykle
    bool hasCycles();

    // symulacja, pojedyńczy krok
    void simulate();
    // symulacja, wiele kroków
    void simulateMultipleSteps(int steps);

    // gettery listy bloków i połączeń
    const std::vector<std::unique_ptr<Block>>& getBlocks() const;
    const std::vector<Connection>& getConnections() const;
};


#endif