//
// Created by tajbe on 24.03.2025.
//

#include "blocks.h"

// w tym pliku są implementacje specyficznych bloków



// ----------------------------------------------------------------------------------------------------------------------------------------------
// sumowania
SumBlock::SumBlock(int _id) : Block(_id, 2, 1) {}

void SumBlock::process() {
    outputValues[0] = inputValues[0] + inputValues[1];
    std::cout<<"sum: "<<inputValues[0]<<" + "<<inputValues[1]<<" = "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void SumBlock::drawContent() {
    ImGui::Text("Sum: %f", outputValues[0]);
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// mnożenia
MultiplyBlock::MultiplyBlock(int _id) : Block(_id, 2, 1) {}

void MultiplyBlock::process() {
    outputValues[0] = inputValues[0] * inputValues[1];
    std::cout<<"multiply: "<<inputValues[0]<<" * "<<inputValues[1]<<" = "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void MultiplyBlock::drawContent() {
    ImGui::Text("Mult: %f", outputValues[0]);
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// całkowania
IntegratorBlock::IntegratorBlock(int _id, double dt) : Block(_id, 1, 1), state(0.0), timeStep(dt) {}

void IntegratorBlock::process() {
    state += inputValues[0] * timeStep;
    outputValues[0] = state;
    std::cout<<"integrator: "<<inputValues[0]<<" * "<<timeStep<<" = "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void IntegratorBlock::drawContent() {
    ImGui::Text("Integrator: %f", outputValues[0]);
    ImGui::Text("Time step: ");
    ImGui::InputDouble("", &timeStep);
}

void IntegratorBlock::reset() {
    state = 0.0;
}

void IntegratorBlock::setState(double initialState) {
    state = initialState;
    outputValues[0] = state;
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// input'u
InputBlock::InputBlock(int _id) : Block(_id, 0, 1) {}

void InputBlock::process() {
    outputValues[0] = inputValue;
    std::cout<<"input: "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void InputBlock::drawContent() {
    ImGui::Text("Input: ");
    ImGui::InputDouble("", &inputValue);
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// print'a
PrintBlock::PrintBlock(int _id) : Block(_id, 1, 0) {}

void PrintBlock::process() {
    std::cout << "print: " << inputValues[0] << std::endl;
}

// TODO: GUI
void PrintBlock::drawContent() {
    ImGui::Text("Print: %f", inputValues[0]);
}
