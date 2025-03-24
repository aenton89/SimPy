//
// Created by tajbe on 24.03.2025.
//

#include "blocks.h"

// w tym pliku są implementacje specyficznych bloków



// sumowania
SumBlock::SumBlock(const std::string& name) : Block(name, 2, 1) {}

void SumBlock::process() {
    outputValues[0] = inputValues[0] + inputValues[1];
    std::cout<<"sum: "<<inputValues[0]<<" + "<<inputValues[1]<<" = "<<outputValues[0]<<std::endl;
}


// mnożenia
MultiplyBlock::MultiplyBlock(const std::string& name) : Block(name, 2, 1) {}
void MultiplyBlock::process() {
    outputValues[0] = inputValues[0] * inputValues[1];
    std::cout<<"multiply: "<<inputValues[0]<<" * "<<inputValues[1]<<" = "<<outputValues[0]<<std::endl;
}


// całkowania
IntegratorBlock::IntegratorBlock(const std::string& name, double dt)
    : Block(name, 1, 1), state(0.0), timeStep(dt) {}

void IntegratorBlock::process() {
    state += inputValues[0] * timeStep;
    outputValues[0] = state;
    std::cout<<"integrator: "<<inputValues[0]<<" * "<<timeStep<<" = "<<outputValues[0]<<std::endl;
}

void IntegratorBlock::reset() {
    state = 0.0;
}

void IntegratorBlock::setState(double initialState) {
    state = initialState;
    outputValues[0] = state;
}