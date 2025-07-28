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



// ----------------------------------------------------------------------------------------------------------------------------------------------
// plot'a
PlotBlock::PlotBlock(int _id) : Block(_id, 1, 0) {}

void PlotBlock::process() {
    data[values_offset] = inputValues[0];
    values_offset++;
    std::cout<<"plot's new value: "<<inputValues[0]<<std::endl;
}

void PlotBlock::reset() {
    values_offset = 0;
    // resetujemy dane wykresu
    std::fill(data, data + 1000, 0.0f);
}

void PlotBlock::drawContent() {
    ImVec2 size = ImGui::GetContentRegionAvail();

    if (ImPlot::BeginPlot("##Plot", size, ImPlotFlags_NoLegend | ImPlotFlags_NoMenus)) {
        // Oś X: przesuwamy okno przesuwne na końcówkę danych
        ImPlot::SetupAxisLimits(ImAxis_X1, values_offset - 1000, values_offset, ImGuiCond_Always);
        // Oś Y: zakres 0..1
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0f, 1000.0f, ImGuiCond_Always);

        // rysowanie danych
        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 10.0f);
        ImPlot::PlotLine("##Data", data, 1000, 1, 0, values_offset * sizeof(float));
        ImPlot::PopStyleVar();

        ImPlot::EndPlot();
    }
}

// ----------------------------------------------------------------------------------------------------------------------------------------------
// gain'a
GainBlock::GainBlock(int _id) : Block(_id, 1, 1) {}

void GainBlock::process() {
    outputValues[0] = inputValues[0] * multiplier;
    std::cout<<"gain: "<<inputValues[0]<<" * "<<multiplier<<" = "<<outputValues[0]<<std::endl;
}

void GainBlock::drawContent() {
    ImGui::Text("Multiplier: ");
    ImGui::InputFloat("", &multiplier);
    ImGui::Text("Gain: %f", outputValues[0]);

    // przycisk z trzema kropkami
    ImGui::SameLine();
    if (ImGui::Button("...")) {
        ImGui::OpenPopup("MoreOptionsPopup");
    }

    if (ImGui::BeginPopup("MoreOptionsPopup")) {
        // tu dodatkowe widgety, np. InputText, Checkboxes, Selectable itp.

        static char text[128] = "";
        ImGui::InputText("Enter something", text, IM_ARRAYSIZE(text));

        static bool check = false;
        ImGui::Checkbox("Enable feature", &check);

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}