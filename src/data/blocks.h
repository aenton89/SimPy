//
// Created by tajbe on 24.03.2025.
//

#ifndef BLOCKS_H
#define BLOCKS_H

#include "structures.h"

// w tym pliku są deklaracje specyficznych bloków (narazie testowe)


// bloczek sumujący
class SumBlock : public Block {
public:
    SumBlock(const std::string& name);
    void process() override;
};

// bloczek mnożący
class MultiplyBlock : public Block {
public:
    MultiplyBlock(const std::string& name);
    void process() override;
};

// bloczek całkujący
class IntegratorBlock : public Block {
private:
    double state;
    double timeStep;

public:
    IntegratorBlock(const std::string& name, double dt);
    void process() override;
    void reset();
    void setState(double initialState);
};


#endif
