//
// Created by tajbe on 24.03.2025.
//

#include "blocks.h"

// w tym pliku są implementacje specyficznych bloków



// ----------------------------------------------------------------------------------------------------------------------------------------------
// sumowania
SumBlock::SumBlock(int _id) : Block(_id, 2, 1, true) {
    size = ImVec2(150, 80);
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
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        if (numInputs < 2)
            numInputs = 2;
        else {
            std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
            inputValues.resize(numInputs);
            negate_inputs.resize(numInputs, 0);
        }
    }

    if (ImGui::Button("Negate Inputs >")) {
        ImGui::OpenPopup("MoreOptionsPopup");
    }

    // samo menu
    if (ImGui::BeginPopup("MoreOptionsPopup")) {
        for (int i = 0; i < numInputs; ++i) {
            // rzutujemy char* na bool* - to działa bo bool i char są 1-bajtowe
            ImGui::Checkbox(("Negate Input " + std::to_string(i + 1)).c_str(), reinterpret_cast<bool*>(&negate_inputs[i]));
        }

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SumBlock::resetBefore() {
    std::fill(inputValues.begin(), inputValues.end(), 0);
    std::fill(outputValues.begin(), outputValues.end(), 0);
}


// ----------------------------------------------------------------------------------------------------------------------------------------------
// mnożenia
MultiplyBlock::MultiplyBlock(int _id) : Block(_id, 2, 1, true) {
    size = ImVec2(150, 80);
}

void MultiplyBlock::process() {
    outputValues[0] = inputValues[0] * inputValues[1];
    std::cout<<"multiply: "<<inputValues[0]<<" * "<<inputValues[1]<<" = "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void MultiplyBlock::drawContent() {
    ImGui::Text("Mult: %f", outputValues[0]);

    Block::drawContent();
}

void MultiplyBlock::drawMenu() {
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        if (numInputs < 2)
            numInputs = 2;
        else {
            std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
            inputValues.resize(numInputs);
        }
    }
}

void MultiplyBlock::resetBefore() {
    std::fill(inputValues.begin(), inputValues.end(), 0);
    std::fill(outputValues.begin(), outputValues.end(), 0);
}


// ----------------------------------------------------------------------------------------------------------------------------------------------
// całkowania
IntegratorBlock::IntegratorBlock(int _id, double dt) : Block(_id, 1, 1, true), initial_state(0.0), timeStep(dt) {
    size = ImVec2(200, 120);
    state = initial_state;
}

void IntegratorBlock::process() {
    state += inputValues[0] * timeStep;
    outputValues[0] = state;
    std::cout<<"integrator: "<<inputValues[0]<<" * "<<timeStep<<" = "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void IntegratorBlock::drawContent() {
    ImGui::Text("Time step: ");
    ImGui::InputDouble("", &timeStep);
    ImGui::Text("Integrator: %f", outputValues[0]);

    Block::drawContent();
}

void IntegratorBlock::resetAfter() {
    state = initial_state;
}

void IntegratorBlock::resetBefore() {
    state = initial_state;
    outputValues[0] = initial_state;
}

void IntegratorBlock::setState(double _initial_state) {
    initial_state = _initial_state;
    outputValues[0] = _initial_state;
}

void IntegratorBlock::drawMenu() {
    ImGui::InputDouble("Initial state: ", &initial_state);
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// input'u
InputBlock::InputBlock(int _id) : Block(_id, 0, 1) {
    size = ImVec2(150, 80);
    inputValue = 0.0;
}

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
PrintBlock::PrintBlock(int _id) : Block(_id, 1, 0) {
    size = ImVec2(150, 60);
}

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
PlotBlock::PlotBlock(int _id) : Block(_id, 1, 0, true) {
    size = ImVec2(350, 200);
    data.resize(numInputs);
    for (auto& arr : data) {
        std::fill(arr.begin(), arr.end(), 0.0f);
    }
}

void PlotBlock::process() {
    if (data.size() < numInputs)
        return;

    for (int i = 0; i < numInputs; ++i) {
        if (max_val < inputValues[i])
            max_val = inputValues[i];
        else if (min_val > inputValues[i])
            min_val = inputValues[i];

        data.at(i)[values_offset] = inputValues[i];
        std::cout<<"plot's new value: "<<inputValues[i]<<std::endl;
    }

    values_offset++;
    if (values_offset > 1000)
        values_offset = 1000;
}

void PlotBlock::resetBefore() {
    std::fill(inputValues.begin(), inputValues.end(), 0);

    values_offset = 0;
    min_val = -1.0f;
    max_val = 1.0f;
    for (int i = 0; i < numInputs; ++i) {
        // resetujemy dane wykresu
        std::fill(data.at(i).begin(), data.at(i).end(), 0.0f);
    }
}

void PlotBlock::drawContent() {
    Block::drawContent();

    ImVec2 size = ImGui::GetContentRegionAvail();

    if (ImPlot::BeginPlot("##Plot", size, ImPlotFlags_NoLegend | ImPlotFlags_NoMenus)) {
        // Oś X: przesuwamy okno przesuwne na końcówkę danych
        ImPlot::SetupAxisLimits(ImAxis_X1, values_offset - 1000, values_offset, ImGuiCond_Always);
        // Oś Y: zakres 0..1
        ImPlot::SetupAxisLimits(ImAxis_Y1, min_val, max_val + (max_val / 10.0), ImGuiCond_Always);

        for (int i = 0; i < numInputs; ++i) {
            if (i >= data.size())
                continue;
            ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
            // unikalna etykieta
            std::string label = "Input " + std::to_string(i + 1);
            ImPlot::PlotLine(label.c_str(), data[i].data(), 1000, 1, 0, values_offset * sizeof(float));
            ImPlot::PopStyleVar();
        }
        ImPlot::EndPlot();
    }
}

void PlotBlock::drawMenu() {
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        if (numInputs < 1)
            numInputs = 1;
        else {
            std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
            data.resize(numInputs);
            inputValues.resize(numInputs);
            for (int i = 0; i < numInputs; ++i) {
                std::fill(data[i].begin(), data[i].end(), 0.0f);
            }
        }
    }
}

// ----------------------------------------------------------------------------------------------------------------------------------------------
// gain'a
GainBlock::GainBlock(int _id) : Block(_id, 1, 1, true) {
    size = ImVec2(200, 120);
}

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

void GainBlock::resetBefore() {
    std::fill(inputValues.begin(), inputValues.end(), 0);
    std::fill(outputValues.begin(), outputValues.end(), 0);
}

// ----------------------------------------------------------------------------------------------------------------------------------------------
// data sending
DataSenderBlock::DataSenderBlock(int id, const std::string& channel)
    : Block(id, 1, 0), channelName(channel), dataTypeName("float"), sendEnabled(true), sendCounter(0) {
    size = ImVec2(250, 150);
}

void DataSenderBlock::process() {
    if (!sendEnabled) return;

    // wysyłanie w zależności od wybranego typu
    if (dataTypeName == "float") {
        DataChannelManager::sendData<float>(channelName, inputValues[0], "float");
    }
    else if (dataTypeName == "int") {
        DataChannelManager::sendData<int>(channelName, static_cast<int>(inputValues[0]), "int");
    }
    else if (dataTypeName == "double") {
        DataChannelManager::sendData<double>(channelName, static_cast<double>(inputValues[0]), "double");
    }

    sendCounter++;
    std::cout << "Sent to '" << channelName << "': " << inputValues[0]
              << " (type: " << dataTypeName << ", count: " << sendCounter << ")" << std::endl;
}

void DataSenderBlock::drawContent() {
    ImGui::Text("Data Sender");

    // nazwa kanału
    char channelBuffer[128];
    strcpy(channelBuffer, channelName.c_str());
    if (ImGui::InputText("Channel", channelBuffer, sizeof(channelBuffer))) {
        channelName = std::string(channelBuffer);
    }

    // typ danych
    const char* types[] = {"float", "int", "double"};
    static int currentType = 0;
    if (ImGui::Combo("Data Type", &currentType, types, IM_ARRAYSIZE(types))) {
        dataTypeName = types[currentType];
    }

    // kontrolki
    ImGui::Checkbox("Send enabled", &sendEnabled);

    // informacje
    ImGui::Separator();
    ImGui::Text("Queue size: %d", DataChannelManager::getChannelSize(channelName));
    ImGui::Text("Sent count: %d", sendCounter);

    // lista dostępnych kanałów
    ImGui::Text("Available channels:");
    auto channels = DataChannelManager::getAvailableChannels();
    for (const auto& ch : channels) {
        ImGui::Text("  - %s (%d)", ch.c_str(), DataChannelManager::getChannelSize(ch));
    }

    // przycisk czyszczenia
    if (ImGui::Button("Clear Channel")) {
        DataChannelManager::clearChannel(channelName);
    }

    Block::drawContent();
}

// getters/setters
void DataSenderBlock::setChannelName(const std::string& name) {
    channelName = name;
}

std::string DataSenderBlock::getChannelName() const {
    return channelName;
}

void DataSenderBlock::setDataType(const std::string& type) {
    dataTypeName = type;
}

std::string DataSenderBlock::getDataType() const {
    return dataTypeName;
}

bool DataSenderBlock::isSendEnabled() const {
    return sendEnabled;
}

void DataSenderBlock::setSendEnabled(bool enabled) {
    sendEnabled = enabled;
}

int DataSenderBlock::getSendCounter() const {
    return sendCounter;
}