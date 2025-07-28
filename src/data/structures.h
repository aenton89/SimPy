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


/* klasa z której dziedziczą wszystkie bloki, odpowiada za:
- procesowanie danych
- przechowywanie danych wejściowych i wyjściowych
- liczbe wejść i wyjść
- nazwe bloku
 */
class Block {
protected:
    // std::string name;
    std::vector<double> inputValues;
    std::vector<double> outputValues;
    int numInputs;
    int numOutputs;

public:
    // TODO: GUI - zmienne od GUI
    int id;
    ImVec2 position = ImVec2(100, 100);
    ImVec2 size = ImVec2(300, 120);
    bool open = true;
    std::vector<int> connections;
    int numConnected = 0;

    Block(int _id, int _numInputs, int _numOutputs);
    virtual ~Block() = default;

    // TODO: GUI - metoda do rysowania
    virtual void drawContent() = 0;

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
    virtual void reset() {};
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

    // czyszczenie na koniec symulacji
    void cleanup();

    // gettery listy bloków i połączeń
    std::vector<std::unique_ptr<Block>>& getBlocks();
    std::vector<Connection>& getConnections();

    // TODO: łączenie w całość
    void makeConnections();
    // dodawanie i usuwanie bloków
    void addBlock(std::unique_ptr<Block> block);
    void removeBlock(int removeId);
};


#endif