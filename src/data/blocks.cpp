//
// Created by tajbe on 24.03.2025.
//

#include "blocks.h"

// w tym pliku są implementacje specyficznych bloków



// ----------------------------------------------------------------------------------------------------------------------------------------------
// sumowania
SumBlock::SumBlock(int _id) : Block(_id, 2, 1, true) {
    if (numInputs != 2) {
        negate_inputs.resize(numInputs, 0);
    }
}

void SumBlock::process() {
    outputValues[0] = 0.0;
    for (int i = 0; i < numInputs; i++) {
        if (negate_inputs[i]) {
            inputValues[i] = -inputValues[i];
        }

        outputValues[0] += inputValues[i];
    }
    std::cout<<"sum: "<<inputValues[0]<<" + "<<inputValues[1]<<" = "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void SumBlock::drawContent() {
    ImGui::Text("Sum: %f", outputValues[0]);

    Block::drawContent();
}

void SumBlock::drawMenu() {
    for (int i = 0; i < numInputs; ++i) {
        // rzutujemy char* na bool* - to działa bo bool i char są 1-bajtowe
        ImGui::Checkbox(("Negate Input " + std::to_string(i + 1)).c_str(), reinterpret_cast<bool*>(&negate_inputs[i]));
    }
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// mnożenia
MultiplyBlock::MultiplyBlock(int _id) : Block(_id, 2, 1, true) {}

void MultiplyBlock::process() {
    outputValues[0] = inputValues[0] * inputValues[1];
    std::cout<<"multiply: "<<inputValues[0]<<" * "<<inputValues[1]<<" = "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void MultiplyBlock::drawContent() {
    ImGui::Text("Mult: %f", outputValues[0]);

    Block::drawContent();
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// całkowania
IntegratorBlock::IntegratorBlock(int _id, double dt) : Block(_id, 1, 1, true), state(0.0), timeStep(dt) {}

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

    Block::drawContent();
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

    Block::drawContent();
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

    Block::drawContent();
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// plot'a
PlotBlock::PlotBlock(int _id) : Block(_id, 1, 0) {}

void PlotBlock::process() {
    if (max_val < inputValues[0])
        max_val = inputValues[0];
    else if (min_val > inputValues[0])
        min_val = inputValues[0];

    data[values_offset] = inputValues[0];
    values_offset++;
    std::cout<<"plot's new value: "<<inputValues[0]<<std::endl;
}

void PlotBlock::reset() {
    values_offset = 0;
    min_val = -1.0f;
    max_val = 1.0f;
    // resetujemy dane wykresu
    std::fill(data, data + 1000, 0.0f);
}

void PlotBlock::drawContent() {
    ImVec2 size = ImGui::GetContentRegionAvail();

    if (ImPlot::BeginPlot("##Plot", size, ImPlotFlags_NoLegend | ImPlotFlags_NoMenus)) {
        // Oś X: przesuwamy okno przesuwne na końcówkę danych
        ImPlot::SetupAxisLimits(ImAxis_X1, values_offset - 1000, values_offset, ImGuiCond_Always);
        // Oś Y: zakres 0..1
        ImPlot::SetupAxisLimits(ImAxis_Y1, min_val, max_val + (max_val / 10.0), ImGuiCond_Always);

        // rysowanie danych
        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 10.0f);
        ImPlot::PlotLine("##Data", data, 1000, 1, 0, values_offset * sizeof(float));
        ImPlot::PopStyleVar();

        ImPlot::EndPlot();
    }

    Block::drawContent();
}

// ----------------------------------------------------------------------------------------------------------------------------------------------
// gain'a
GainBlock::GainBlock(int _id) : Block(_id, 1, 1, true) {}

void GainBlock::process() {
    outputValues[0] = inputValues[0] * multiplier;
    std::cout<<"gain: "<<inputValues[0]<<" * "<<multiplier<<" = "<<outputValues[0]<<std::endl;
}

void GainBlock::drawContent() {
    ImGui::Text("Multiplier: ");
    ImGui::InputFloat("", &multiplier);
    ImGui::Text("Gain: %f", outputValues[0]);

    Block::drawContent();
}

void GainBlock::drawMenu() {
    // tu dodatkowe widgety, np. InputText, Checkboxes, Selectable itp.
    static char text[128] = "";
    ImGui::InputText("Enter something", text, IM_ARRAYSIZE(text));

    static bool check = false;
    ImGui::Checkbox("Enable feature", &check);
}
