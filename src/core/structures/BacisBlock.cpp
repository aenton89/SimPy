//
// Created by tajbe on 24.03.2025.
//

#include "BasicBlock.h"



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

