//
// Created by tajbe on 24.03.2025.
//

#ifndef BLOCKS_H
#define BLOCKS_H

#include "structures.h"
#include <implot.h>

// w tym pliku są deklaracje specyficznych bloków (narazie testowe)



// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek sumujący
class SumBlock : public Block {
    // bo vector<bool> nie zwraca zwykłego wskaźnika (chyba??)
    std::vector<char> negate_inputs = {0, 0};
public:
    SumBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void drawMenu() override;
};



// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek mnożący
class MultiplyBlock : public Block {
public:
    MultiplyBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
};



// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek całkujący
class IntegratorBlock : public Block {
private:
    double state;
    double timeStep;

public:
    // default'owo dt = 0.01
    IntegratorBlock(int _id, double dt = 0.01);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void reset();
    void setState(double initialState);
};



// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek input'u
class InputBlock : public Block {
private:
    double inputValue;

public:
    InputBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
};



// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek print'a
class PrintBlock : public Block {
public:
    PrintBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek robiący wykres
class PlotBlock : public Block {
    int values_offset = 0; // offset do przesunięcia wykresu
    float data[1000] = {}; // wskaźnik do danych do wykresu
    float max_val = 1.0f;
    float min_val = -1.0f;
public:
    PlotBlock(int _id);
    void process() override;
    void drawContent() override;
    void reset() override;
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// jak MultiplyBlock, ale z ustawianym mnożnikiem
class GainBlock: public Block {
    float multiplier = 1.0f;
public:
    GainBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void drawMenu() override;
};

#endif
