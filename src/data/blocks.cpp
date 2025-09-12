//
// Created by tajbe on 24.03.2025.
//

#include "blocks.h"
#include "data_sender/data_channel_manager.h"
#include "math/digital_signal_processing/DSP.h"
#include "math/math_help_fun/math_help_fun.h"
#include <sstream>
#include <cmath>
#include <numbers>
#include <complex>
#include <algorithm>

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
IntegratorBlock::IntegratorBlock(int _id) : Block(_id, 1, 1, true), initial_state(0.0) {
    size = ImVec2(200, 120);

    ss.A = {{0.0}};
    ss.B = {{1.0}};
    ss.C = {{1.0}};
    ss.D = {{0.0}};
    ss.x = {initial_state};
}

void IntegratorBlock::process() {
    // state += inputValues[0] * timeStep;
    // outputValues[0] = state;
    auto solver = SolverManager::solver();
    if (!solver) return;

    std::vector<double> uvec = { inputValues[0] };
    solver->step(ss, uvec);
    outputValues[0] = ss.x[0];

    std::vector<double> yvec = MatOp::matVecMul(ss.C, ss.x);
    double y = yvec[0] + ss.D[0][0] * inputValues[0];
    outputValues[0] = y;

    //std::cout<<"integrator: "<<inputValues[0]<<" * "<<timeStep<<" = "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void IntegratorBlock::drawContent() {
    ImGui::Text("IntegratorBlock");
    Block::drawContent();
}

void IntegratorBlock::resetAfter() {

}

void IntegratorBlock::resetBefore() {
    ss.x = {initial_state};
    outputValues[0] = initial_state;
}

void IntegratorBlock::setState(double _initial_state) {
    initial_state = _initial_state;
    outputValues[0] = _initial_state;
}

void IntegratorBlock::drawMenu() {
    ImGui::InputDouble("Initial state: ", &initial_state);
    ImGui::InputDouble("Time step: ", &Model::timeStep);
}



// ------------------------------------------------------------------------------------------------------------------------------------
// blok pierwsikaonia
sqrtBlock::sqrtBlock(int id_): Block(id_, 1, 1, true)
{
    size = ImVec2(150, 80);
}

void sqrtBlock::process()
{
    if (mode == "absolut value")
        outputValues[0] = std::sqrt(std::abs(inputValues[0]));
    else if (mode == "equal 0")
    {
        if (inputValues[0] < 0.0)
            inputValues[0] = 0.0;
        outputValues[0] = std::sqrt(inputValues[0]);
    }
    // TODO Trzeba dorobic takz zeby prznosl sie wartosc rzeczywista i zeoslona i zeby te wartsoci byly np ploktowane na wykresie jako dwie ine wykresy
    else if (mode == "complex value")
    {

    }
}

void sqrtBlock::drawContent()
{
    ImGui::Text("Sqrt");
    Block::drawContent();
}

void sqrtBlock::drawMenu()
{
    const static char* sqrt_mode[] = {"absolut value", "equal 0", "complex value"};
    static int current_selected = 0;

    if (ImGui::BeginCombo("Sqrt mode", sqrt_mode[current_selected]))
    {
        for (int i = 0; i < IM_ARRAYSIZE(sqrt_mode); i++)
        {
            bool is_selected = (current_selected == i);
            if (ImGui::Selectable(sqrt_mode[i], is_selected))
            {
                current_selected = i;
                this->mode = sqrt_mode[i];
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}


// ----------------------------------------------------------------------------------------------------------------------------------------------
// input'u
StepBlock::StepBlock(int _id) : Block(_id, 0, 1, true) {
    size = ImVec2(150, 80);
    inputValue = 0.0;
}

void StepBlock::process() {
    if (currentTime < delay)
        outputValues[0] = 0;
    else
        outputValues[0] = inputValue;
    currentTime += Model::timeStep;
    //std::cout<<"input: "<<outputValues[0]<<std::endl;
}

// TODO: GUI
void StepBlock::drawContent() {
    ImGui::Text("Step Input");
    Block::drawContent();
}

void StepBlock::drawMenu() {
    ImGui::InputDouble("Dealy", &delay);
    ImGui::InputDouble("Value", &inputValue);
}

void StepBlock::resetBefore() {
    currentTime = 0;
}



// ---------------------------------------------------------------------------------------------------------------------------------------------
// bloczek inputu w postaci Sinusa
SinusInputBlock::SinusInputBlock(int id_) : Block(id_, 0, 1, true) {
    size = ImVec2(150, 80);
    inputValue = 0.0;
}

void SinusInputBlock::process() {
    // A*sin(2*pi*f + phase)
    outputValues[0] = amplitude*std::sin(2*std::numbers::pi*frequency*currentTime + shiftPhase);
    currentTime += Model::timeStep;
}

void SinusInputBlock::resetBefore() {
    currentTime = 0;
}

void SinusInputBlock::drawContent() {
    ImGui::Text("Sinus Input");
    Block::drawContent();
}

void SinusInputBlock::drawMenu() {
    ImGui::Text("Amp * sin(2*pi*Freq*t + Phase)");
    ImGui::InputDouble("Amp", &amplitude);
    ImGui::InputDouble("Freq", &frequency);
    ImGui::InputDouble("Phase", &shiftPhase);
}



// ---------------------------------------------------------------------------------------------------------------------------------------------
// PWM input
PWMInputBlock::PWMInputBlock(int _id) : Block(_id, 0, 1, true) {
    size = ImVec2(150, 200);
}

void PWMInputBlock::drawMenu() {
    ImGui::InputDouble("Delay", &delay);
    ImGui::InputDouble("Value", &inputValue);
    ImGui::InputDouble("freq", &frequency);
    ImGui::InputDouble("Duty Cycle", &dutyCycle);
}

void PWMInputBlock::drawContent() {
    ImGui::Text("PWM Generator");
    Block::drawContent();
}

void PWMInputBlock::process() {
    double T = 1.0 / this->frequency;  // okres

    if (this->currentTime < this->delay)
        this->outputValues[0] = 0;
    // amplituda
    else if (std::fmod(this->currentTime, T) < this->dutyCycle * T)
        this->outputValues[0] = this->inputValue;
    else
        this->outputValues[0] = 0;

    this->currentTime += Model::timeStep;
}


void PWMInputBlock::resetBefore() {
    currentTime = 0;
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// Generator bilalego szumu
WhiteNoiseInputBlock::WhiteNoiseInputBlock(int id_) : Block(id_, 0, 1, true), distribution(mean, std) {
    size = ImVec2(150, 80);
    std::random_device rd;
    generator.seed(rd());
}

void WhiteNoiseInputBlock::process() {
    outputValues[0] = distribution(generator);
}

void WhiteNoiseInputBlock::drawMenu() {
    if (ImGui::InputDouble("Mean", &mean))
        distribution = std::normal_distribution<double>(mean, std);
    if (ImGui::InputDouble("Std", &std))
        distribution = std::normal_distribution<double>(mean, std);
    if (ImGui::InputDouble("Seed", &seed));
        generator.seed(seed);
}

void WhiteNoiseInputBlock::drawContent() {
    ImGui::Text("White Noise Generator");
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
    if (data.size() < numInputs) return;

    for (int i = 0; i < numInputs; ++i) {
        // aktualizacja min/max Y
        if (y_limMax < inputValues[i]) y_limMax = inputValues[i];
        if (y_limMin > inputValues[i]) y_limMin = inputValues[i];

        // dodajemy nową wartość na końcu wektora
        data[i].push_back(inputValues[i]);

        std::cout << "plot's new value: " << inputValues[i] << std::endl;
    }

    // x_limMax teraz jest po prostu długością danych
    x_limMax = Model::simTime;
}


void PlotBlock::resetBefore() {
    std::fill(inputValues.begin(), inputValues.end(), 0);

    x_limMax = 0;
    y_limMin = -1.0f;
    y_limMax = 1.0f;
    for (int i = 0; i < numInputs; ++i) {
        // resetujemy dane wykresu
        data[i].clear();
    }
}

void PlotBlock::drawContent() {
    Block::drawContent();

    ImVec2 size = ImGui::GetContentRegionAvail();

    if (ImPlot::BeginPlot("##Plot", size, ImPlotFlags_NoLegend)) {
        // Oś X: przesuwamy okno przesuwne na końcówkę danych
        ImPlot::SetupAxisLimits(ImAxis_X1, x_limMin, x_limMax, ImGuiCond_Always);
        // Oś Y: zakres 0..1
        ImPlot::SetupAxisLimits(ImAxis_Y1, y_limMin, y_limMax + (y_limMax / 10.0), ImGuiCond_Always);

        for (int i = 0; i < numInputs; ++i) {
            if (i >= data.size()) continue;
            ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
            std::string label = "Input " + std::to_string(i + 1);
            ImPlot::PlotLine(label.c_str(), data[i].data(), data[i].size(), Model::timeStep);
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
            std::cout << "changed number of inputs: " << numInputs << std::endl;
            data.resize(numInputs);
            inputValues.resize(numInputs);
            for (int i = 0; i < numInputs; ++i) {
                data[i].clear();
            }
        }
    }

    // Limity osi X
    ImGui::InputFloat("X min", &x_limMin);
    ImGui::SameLine();
    ImGui::InputFloat("X max", &x_limMax);

    // Limity osi Y
    ImGui::InputFloat("Y min", &y_limMin);
    ImGui::SameLine();
    ImGui::InputFloat("Y max", &y_limMax);

    // Reset
    if (ImGui::Button("Reset axis")) {
        x_limMin = x_limMax = 0;
        y_limMin = y_limMax = 0;
    }
}



// --------------------------------------------------------------------------------------------------------------------------------------
//  plotowanie grafu XY
// trzeba to pobrac. Array trzeba zamienic na dynamiczny wketyor, 2 wejścia: X i Y
PLotXYBlock::PLotXYBlock(int _id) : Block(_id, 2, 0, true) {
    size = ImVec2(350, 200);
    data.resize(2); // X i Y
    for (auto &arr : data) {
        arr.fill(0.0f);
    }
}

void PLotXYBlock::process() {
    // Musimy mieć oba wejścia (X i Y)
    if (inputValues.size() < 2)
        return;

    float x = inputValues[0];
    float y = inputValues[1];

    // Aktualizacja limitów osi X i Y
    if (sampleIndex == 0) {
        x_limMin = x_limMax = x;
        y_limMin = y_limMax = y;
    }
    else {
        if (x < x_limMin) x_limMin = x;
        if (x > x_limMax) x_limMax = x;
        if (y < y_limMin) y_limMin = y;
        if (y > y_limMax) y_limMax = y;
    }

    // Zapisz nową próbkę
    if (sampleIndex < (int)data[0].size()) {
        data[0][sampleIndex] = x;
        data[1][sampleIndex] = y;
        sampleIndex++;
    }
}

void PLotXYBlock::drawContent()
{
    Block::drawContent();
    ImVec2 size = ImGui::GetContentRegionAvail();

    if (ImPlot::BeginPlot("##PlotXY", size, ImPlotFlags_NoLegend)) {
        ImPlot::SetupAxisLimits(ImAxis_X1, x_limMin, x_limMax, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, y_limMin, y_limMax + (y_limMax / 10.0f), ImGuiCond_Always);

        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
        std::string label = "XY plot";
        // rysujemy sampleIndex punktów
        ImPlot::PlotLine(label.c_str(), data[0].data(), data[1].data(), sampleIndex);
        ImPlot::PopStyleVar();

        ImPlot::EndPlot();
    }
}

void PLotXYBlock::resetBefore() {
    for (auto &arr : data) {
        arr.fill(0.0f);
    }
    sampleIndex = 0;
    x_limMin = y_limMin = -1.0f;
    x_limMax = y_limMax = 1.0f;
}

void PLotXYBlock::drawMenu() {
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        if (numInputs < 1)
            numInputs = 1;
        else {
            std::cout << "changed number of inputs: " << numInputs << std::endl;
            data.resize(numInputs);
            inputValues.resize(numInputs);
            for (int i = 0; i < numInputs; ++i) {
                std::fill(data[i].begin(), data[i].end(), 0.0f);
            }
        }
    }

    // Limity osi X
    ImGui::InputFloat("X min", &x_limMin);
    ImGui::SameLine();
    ImGui::InputFloat("X max", &x_limMax);

    // Limity osi Y
    ImGui::InputFloat("Y min", &y_limMin);
    ImGui::SameLine();
    ImGui::InputFloat("Y max", &y_limMax);

    // Reset
    if (ImGui::Button("Reset axis")) {
        x_limMin = x_limMax = 0;
        y_limMin = y_limMax = 0;
    }
}


// --------------------------------------------------------------------------------------------------------------------------------------
// plot heatmap
PlotHeatmapBlock::PlotHeatmapBlock(int _id) : Block(_id, 65, 0, false){
    size = ImVec2(350, 200);
}

void PlotHeatmapBlock::process() {
    if (!std::isnan(inputValues[0])) {
        std::cout << "------------------HeatMap---------------: " << inputValues[0] << " " << inputValues.size() << std::endl;
        for (auto val: inputValues) {
            std::cout << val << " ";
        }
        num_row = std::max(num_row, inputValues.size());
        data.insert(data.end(), inputValues.begin(), inputValues.end());
    }
}

void PlotHeatmapBlock::drawContent() {
    if (ImPlot::BeginPlot("##PlotHeatmap", size)) {

        int rows = num_row;
        // zakładamy, że data.size() jest wielokrotnością rows
        int cols = data.size() / rows;

        if (cols > 0 && rows > 0) {
            ImPlot::PlotHeatmap(
                "Spectrogram",
                data.data(),
                rows, cols,
                0, 0,                   // skala min/max (kolory)
                "%.1f",                         // format etykiet
                ImPlotPoint(0, 0),          // minimalny punkt (x_min, y_min)
                ImPlotPoint(cols-1, rows-1) // maksymalny punkt (x_max, y_max)
            );
        }

        ImPlot::EndPlot();
    }

    Block::drawContent();


    // int i = 0;
    // for (auto &arr : data) {
    //     std::cout << arr << " ";;
    //     i ++ ;
    //     if (i % num_row == 0)
    //         std::cout << std::endl;
    // }
}

void PlotHeatmapBlock::resetBefore() {
    data.clear();
}


// ---------------------------------------------------------------------------------------------------------------------------------------
// roznczkowanie
DifferentiatorBlock::DifferentiatorBlock(int _id) : Block(_id, 1, 1, true), initial_state(0.0) {}

void DifferentiatorBlock::process() {
    double derivative = (inputValues[0] - state) / Model::timeStep;
    state = inputValues[0];
    outputValues[0] = derivative;

    // std::cout << "differentiator: (" << inputValues[0]
    //           << " - " << initial_state
    //           << ") / " << timeStep
    //           << " = " << outputValues[0]
    //           << std::endl;
}

// TODO: GUI
void DifferentiatorBlock::drawContent() {
    ImGui::Text("Differentiator Block");
    Block::drawContent();
}

void DifferentiatorBlock::resetAfter() {
    state = initial_state;
}

void DifferentiatorBlock::resetBefore() {
    state = initial_state;
    outputValues[0] = initial_state;
}

void DifferentiatorBlock::setState(double _initial_state) {
    initial_state = _initial_state;
    outputValues[0] = _initial_state;
}

void DifferentiatorBlock::drawMenu() {
    ImGui::InputDouble("Initial state: ", &initial_state);
    ImGui::InputDouble("Time step: ", &Model::timeStep);
}


// -----------------------------------------------------------------------------------------------------------------------------------------
// Blok saturacji
SaturationBlock::SaturationBlock(int id_): Block(id_, 1, 1, true) {
    this->size = ImVec2(200, 150);
}

void SaturationBlock::process() {
    this->outputValues[0] = std::clamp(this->inputValues[0], lowerLimit, upperLimit);
}

void SaturationBlock::drawContent() {
    ImGui::Text("Saturation block");
    Block::drawContent();
}

void SaturationBlock::drawMenu() {
    ImGui::InputDouble("Upper limit", &upperLimit);
    ImGui::InputDouble("Lower limit", &lowerLimit);
}



// --------------------------------------------------------------------------------------------------------------------------------------------
// transmitacja operatorowa
TransferFuncionContinous::TransferFuncionContinous(int id_) : Block(id_, 1, 1, true) {
    this->size = ImVec2(200, 130);
    this->num = "1";
    this->denum = "1, 1";
    this->run_tf2ss = true;
}

std::vector<float> TransferFuncionContinous::stringToVector(const std::string& s) {
    std::vector<float> result;
    std::stringstream ss(s);
    std::string item;

    // dzieli po przecinku
    while (std::getline(ss, item, ',')) {
        try {
            // usuń spacje z początku i końca
            size_t start = item.find_first_not_of(" \t");
            size_t end = item.find_last_not_of(" \t");
            std::string trimmed = (start == std::string::npos) ? "" : item.substr(start, end - start + 1);

            // konwertuj na float
            if (!trimmed.empty())
                result.push_back(std::stof(trimmed));
        } catch (...) {
            // jeśli coś nie jest liczbą, pomiń
        }
    }

    return result;
}


std::string floatToStringTrimmed(float value) {
    // usuwanie zbednych zer w float
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::string polyToString(const std::vector<float>& coeffs) {
    std::string result;
    int n = coeffs.size();

    for (int i = 0; i < n; ++i) {
        float coeff = coeffs[i];
        int power = n - i - 1;

        if (coeff == 0)
            continue;

        if (!result.empty())
            result += " + ";

        if (power == 0) {
            result += floatToStringTrimmed(coeff);
        }
        else if (power == 1) {
            if (coeff == 1)
                result += "s";
            else
                result += floatToStringTrimmed(coeff) + "s";
        }
        else {
            if (coeff == 1)
                result += "s^" + std::to_string(power);
            else
                result += floatToStringTrimmed(coeff) + "s^" + std::to_string(power);
        }
    }

    return result.empty() ? "0" : result;
}

void TransferFuncionContinous::drawContent() {
    ImGui::Text("Transfer Funcion Continous");
    ImGui::Separator();
    std::string num_str = polyToString(this->numerator).c_str();
    std::string denum_str = polyToString(this->denominator).c_str();

    if (num_str.substr(0, 2) > denum_str.substr(0, 2) ) {
        num_str += " !";
        denum_str += " !";
    }

    ImGui::Text(num_str.c_str());
    ImGui::Text("-------------");
    ImGui::Text(denum_str.c_str());

    if (this->run_tf2ss) {
        this->numerator = stringToVector(num);
        this->denominator = stringToVector(denum);

        // if (this->numerator.size() < this->denominator.size())
        ss = dsp::tf2ss(this->numerator, this->denominator);
        this->run_tf2ss = false;

    }
    Block::drawContent();
}

void TransferFuncionContinous::drawMenu() {
    static char num_buf[128];
    static char denum_buf[128];

    strncpy(num_buf, num.c_str(), sizeof(num_buf));
    strncpy(denum_buf, denum.c_str(), sizeof(denum_buf));

    // aktualizujemy std::string tylko jeśli użytkownik zmienił tekst
    if (ImGui::InputText("Numerator", num_buf, IM_ARRAYSIZE(num_buf)))
        num = num_buf;
    if (ImGui::InputText("Denominator", denum_buf, IM_ARRAYSIZE(denum_buf)))
        denum = denum_buf;

    this->run_tf2ss = true;
}

void TransferFuncionContinous::process() {
    auto solver = SolverManager::solver();
    if (!solver) return;

    std::vector<double> uvec = { inputValues[0] };
    solver->step(ss, uvec);

    std::vector<double> yvec = MatOp::matVecMul(ss.C, ss.x);
    double y = yvec[0] + ss.D[0][0] * inputValues[0];
    outputValues[0] = y;
}



void TransferFuncionContinous::resetBefore() {
    for (int i = 0; i < outputValues.size(); i++) {
        outputValues[i] = 0.0;
    }
    std::fill(ss.x.begin(), ss.x.end(), 0.0);
}

// -----------------------------------------------------------------------------------------------------------------------------------------
// regulator PID
PID_regulator::PID_regulator(int id_) : Block(id_, 1, 1, true) {
    size = ImVec2(200, 150);
    ss.A = {{0}};
    ss.B = {{1}};
    ss.C = {{Ki}};
    ss.D = {{Kp}};
}

void PID_regulator::process() {
    auto solver = SolverManager::solver();

    if (!solver) return;
    std::vector<double> uvec = { inputValues[0] };
    solver->step(ss, uvec);

    std::vector<double> yvec = MatOp::matVecMul(ss.C, ss.x);

    double y = 0.0;

    if (current_mode == 0) {
        y = yvec[0] + ss.D[0][0] * inputValues[0]
            + Kd * (inputValues[0] - state) / Model::timeStep;
        state = inputValues[0];
    }
    else {
        y = yvec[0] + ss.D[0][0] * inputValues[0];
    }

    outputValues[0] = y;
}


void PID_regulator::drawContent() {
    ImGui::Text("PID");
    Block::drawContent();
};

void PID_regulator::drawMenu() {
    const static char* PID_type[] = {"Without filter", "With filter"};
    if (ImGui::BeginCombo("PID type", PID_type[current_mode], false)) {
        for (int i=0; i < IM_ARRAYSIZE(PID_type); i++) {
            bool is_selected = (current_mode == i);
            if (ImGui::Selectable(PID_type[i], is_selected)) {
                current_mode = i;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::InputDouble("Kp", &Kp);
    ImGui::InputDouble("Kd", &Kd);
    ImGui::InputDouble("Ki", &Ki);


    // TODO trzeba to poprawic

    if (current_mode == 0) {
        ss.A = {{0}};
        ss.B = {{1}};
        ss.C = {{Ki}};
        ss.D = {{Kp}};

        ImGui::Text("Kd*s^2 + Kp*s + Ki");
        ImGui::Text("-------------------");
        ImGui::Text("        s");

    }
    else {
        // Tu cos nie dziala idk dlaczego
        ss.A = {{0, 0}, {0, -tau}};
        ss.B = {{1}, {tau}};
        ss.C = {{Kp*Ki, Kp*Kd}};
        ss.D = {{Kp}};
        ImGui::InputDouble("tau", &tau);

        ImGui::Text("         Ki         Kd");
        ImGui::Text("Kp  + -------  +  -------  ");
        ImGui::Text("         s       1 + tau*s ");

    }
};

void PID_regulator::resetBefore() {
    if (current_mode == 0) {
        ss.A = {{0}};
        ss.B = {{1}};
        ss.C = {{Ki}};
        ss.D = {{Kp}};
        ss.x = {0};
    }
    else {
        ss.A = {{0, 0}, {0, -1/tau}};
        ss.B = {{1}, {1/tau}};
        ss.C = {{Ki, -Kd/tau}};
        ss.D = {{Kp + Kd/tau}};
        ss.x = {0, 0};
    }

    state = 0;
}


// ----------------------------------------------------------------------------------------------------------------------------------------
// FFT
STFT_block::STFT_block(int id_) : Block(id_, 1, 65, true) {
    size = ImVec2(200, 120);
    window_vector = STFT_block::generateWindowVector(windowSize, current_window_mode);
}

void STFT_block::drawContent() {
    ImGui::Text("FFT");
    Block::drawContent();
}

std::vector<double> STFT_block::generateWindowVector(int N, int idx) {
    std::vector<double> windowVector(N);

    for (int i=0; i < N; i++) {
        switch (idx) {
        case 0: // Hann
            windowVector[i] = 0.5 * (1 - cos(2 * std::numbers::pi * i / (N - 1)));
            break;
        case 1: // Hamming
            windowVector[i] = 0.54 - 0.46 * cos(2 * std::numbers::pi * i / (N - 1));
            break;
        case 2:
            windowVector[i] = 0.42 - 0.5 * cos(2 * std::numbers::pi  * i / (N - 1)) + 0.08 * cos(4 * std::numbers::pi * i / (N - 1));
            break;
        case 3: // Okno prostokatne
            windowVector[i] = 1;
            break;
        }
    }
    return windowVector;
}

void STFT_block::drawMenu() {
    // wybor rodziaju okna
    const static char* window_type[] = {"Hann", "Hamming", "Blackman", "Rectangular"};
    if (ImGui::BeginCombo("Window type", window_type[current_window_mode], false)) {
        for (int i=0; i < IM_ARRAYSIZE(window_type); i++) {
            bool is_selected = (current_window_mode == i);
            if (ImGui::Selectable(window_type[i], is_selected)) {
                current_window_mode = i;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    // wybor tego co ma blok zwracac
    const static char* return_type[] = {"Real", "Complex", "Magnitude"};
    if (ImGui::BeginCombo("Return type", return_type[current_return_type], false)) {
        for (int i=0; i < IM_ARRAYSIZE(return_type); i++) {
            bool is_selected = (current_return_type == i);
            if (ImGui::Selectable(return_type[i], is_selected)){
                current_return_type = i;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::InputScalar("Window Size: ", ImGuiDataType_S64, &windowSize);
    ImGui::InputScalar("Overlap: ", ImGuiDataType_S64, &overlap);
    ImGui::InputDouble("fs: ", &fs);

    if (windowSize <= 0) {
        windowSize = 128;
    }

    if (windowSize < overlap)
    {
        overlap = windowSize / 2;
    }

    window_vector = STFT_block::generateWindowVector(windowSize, current_window_mode);
    // nextPow2 = std::pow(2, std::ceil(std::log2(windowSize))); // Do Cooley-Tukey
}


void STFT_block::process() {
    // trzeba tu zrobic myk. Dla okna bedacego potego 2 uzywamy algortymu Cooley-Tukey a dla innych troche wolneiusjzego Bluestein
    batch_vector.push_back(std::complex<double>(inputValues[0], 0.0));
    if (batch_vector.size() == windowSize) {
        for (int i=0; i < windowSize; i++) {
            batch_vector[i] = batch_vector[i]*window_vector[i];
        }

        std::vector<std::complex<double>> X = dsp::bluestein(batch_vector);
        X.erase(X.begin() + X.size() / 2 + 1, X.end());
        std::vector<double> output(X.size());

        // trzba dodac konwersji na poddstwie typu danych ajkie zwracamy
        if (current_return_type == 0) {
            for (size_t i = 0; i < X.size(); i++) {
                output[i] = X[i].real();
            }
        } else if (current_return_type == 1) {
            for (size_t i = 0; i < X.size(); i++) {
                output[i] = X[i].imag();
            }
        } else if (current_return_type == 2){
            for (size_t i = 0; i < X.size(); i++)
                output[i] = std::abs(X[i]);
        }

        outputValues = output; // ja bym tutaj dodal jakis triger ktory mowi ze nastepny blok ma przyjac pkt. Np true albo false
       // std::cout << "------------------STFT---------------: " << outputValues[0] << " " << outputValues.size() << std::endl;
        batch_vector.erase(batch_vector.begin(), batch_vector.begin() + windowSize - overlap);
    } else {
        //std::cout << "------------------STFT_Nan---------------" << batch_vector.size() << std::endl;
        outputValues = {NAN};// uspoeledzone podejscie do zmiaaony gdy sie zrobi triger i temaplate ze mozna zmianac typ wysyalnych danych
    }
}

void STFT_block::resetBefore() {
    outputValues = std::vector<double>(NAN);
    batch_vector.clear();
}

// --------------------------------------------------------------------------------------------------------------------------------------
// Projektowanie filtrow

filterInplementationBlock::filterInplementationBlock(int id_) : Block(id_, 1, 1, true){
    size = ImVec2(200, 120);
    Tf = dsp::butterworth(filter_order, current_pass_type, range);

    std::vector<float> num;
    std::vector<float> den;

    // przesztalcenie tf w postaci kanonicznej na postac wieloamianowa
    for (auto val: math::expandPolynomial(Tf.zeros))
        num.push_back(val.real());

    for (auto val: math::expandPolynomial(Tf.poles))
        den.push_back(val.real());

    ss = dsp::tf2ss(num, den);
}

void filterInplementationBlock::drawContent() {
    ImGui::Text("Filter");
    Block::drawContent();
}

void filterInplementationBlock::drawBodePlot(const dsp::Bode& bode) {
    // // --- Wyznacz min/max dla osi Y ---
    // auto minMaxMagnitude = std::minmax_element(bode.magnitude.begin(), bode.magnitude.end());
    // auto minMaxPhase = std::minmax_element(bode.phase.begin(), bode.phase.end());
    //
    // double mag_min = *minMaxMagnitude.first;
    // double mag_max = *minMaxMagnitude.second;
    // double phase_min = *minMaxPhase.first;
    // double phase_max = *minMaxPhase.second;
    //
    // double omega_min = *std::min_element(bode.omega.begin(), bode.omega.end());
    // double omega_max = *std::max_element(bode.omega.begin(), bode.omega.end());
    //
    // // --- Wykres modułu ---
    // if (ImPlot::BeginPlot("Bode Diagram")) {
    //     ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10); // logarytmiczna oś X
    //     ImPlot::SetupAxisLimits(ImAxis_X1, omega_min, omega_max, ImGuiCond_Always);
    //     ImPlot::SetupAxisLimits(ImAxis_Y1, mag_min, mag_max, ImGuiCond_Always);
    //     ImPlot::SetupAxis(ImAxis_X1, "Frequency [rad/s]");
    //     ImPlot::SetupAxis(ImAxis_Y1, "Magnitude [dB]");
    //
    //     ImPlot::PlotLine("Magnitude", bode.omega.data(), bode.magnitude.data(), (int)bode.omega.size());
    //     ImPlot::EndPlot();
    // }
    //
    // // --- Wykres fazy ---
    // if (ImPlot::BeginPlot("Bode Phase")) {
    //     ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
    //     ImPlot::SetupAxisLimits(ImAxis_X1, omega_min, omega_max, ImGuiCond_Always);
    //     ImPlot::SetupAxisLimits(ImAxis_Y1, phase_min, phase_max, ImGuiCond_Always);
    //     ImPlot::SetupAxis(ImAxis_X1, "Frequency [rad/s]");
    //     ImPlot::SetupAxis(ImAxis_Y1, "Phase [deg]");
    //
    //     ImPlot::PlotLine("Phase", bode.omega.data(), bode.phase.data(), (int)bode.omega.size());
    //     ImPlot::EndPlot();
    // }
}



void filterInplementationBlock::drawMenu() {
    // Typ filtra ze wzgledu na rodzaj przetwarzanego syganlu
    const static char* signal_type[] = {"Analog", "Digital"};

    if (ImGui::BeginCombo("Signal Type", signal_type[current_signal_type], false)) {
        for (int i=0; i < IM_ARRAYSIZE(signal_type); i++) {
            bool is_selected = (current_signal_type == i);
            if (ImGui::Selectable(signal_type[i], is_selected)) {
                current_signal_type = i;
            }
        }
        ImGui::EndCombo();
    }

    // Typ filtra ze wzgledu na rodzaj pasma przenoszenia
    const static char* pass_type[] = {"Low-Pass, LPF", "High-Pass, HPF", "Band-Pass, BPF", "Band-Stop, BSF"}; // mozna tu dodac jescze nie liniowe plus te co przepsuzaja srodk lub go blokuja

    if (ImGui::BeginCombo("Pass Type", pass_type[current_pass_type], false)) {
        for (int i=0; i < IM_ARRAYSIZE(pass_type); i++) {
            bool is_selected = (current_pass_type == i);
            if (ImGui::Selectable(pass_type[i], is_selected)) {
                current_pass_type = i;
            }
        }
        ImGui::EndCombo();
    }

    // Podtypy filtrow analogowych
    const static char* analog_type[] = {"Butterworth", "Chebyshev I", "Chebyshev II", "Bessel", "elliptical"};

    if (current_signal_type == 0){
        if (ImGui::BeginCombo("Analog filter type", analog_type[analog_filter_type], false)){
            for (int i=0; i < IM_ARRAYSIZE(analog_type); i++) {
                bool is_selected = (analog_filter_type == i);
                if (ImGui::Selectable(analog_type[i], is_selected)){
                    analog_filter_type = i;
                }
            }
            ImGui::EndCombo();
        }
    }

    // Podtypy filtrow cyfrowych
    const static char* digital_type[] = {"IR", "IIR", "FIR", "FIIR"};

    if (current_signal_type == 1) {
        if (ImGui::BeginCombo("Analog filter type", digital_type[digital_filter_type], false)){
            for (int i=0; i < IM_ARRAYSIZE(digital_type); i++) {
                bool is_selected = (digital_filter_type == i);
                if (ImGui::Selectable(digital_type[i], is_selected)){
                    digital_filter_type = i;
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::InputInt("Order ", &filter_order);
    filter_order = (filter_order < 1) ? 1 : filter_order;
    ImGui::InputFloat("Ripple ", &ripple);

    if (current_pass_type == BPF || current_pass_type == BSF) {
        ImGui::InputDouble("Lower Limit", &lower_limit);
        ImGui::SameLine();
        ImGui::InputDouble("Upper Limit", &higher_limit);
    }
    else {
        ImGui::InputDouble("Lower Limit", &lower_limit);
    }
    lower_limit = (lower_limit > higher_limit) ? 0 : lower_limit;

    range = {lower_limit * 2 * std::numbers::pi, higher_limit * 2 * std::numbers::pi};

    Tf = dsp::butterworth(filter_order, current_pass_type, range);

    std::vector<float> num;
    std::vector<float> den;

    // przesztalcenie tf w postaci kanonicznej na postac wieloamianowa
    for (const auto& val: math::expandPolynomial(Tf.zeros))
        num.push_back(val.real()*Tf.gain);

    for (const auto& val: math::expandPolynomial(Tf.poles))
        den.push_back(val.real());

    ss = dsp::tf2ss(num, den);

    dsp::Bode bode = dsp::bode_characteristic(Tf);

    filterInplementationBlock::drawBodePlot(bode);

    //dsp::printStateSpace(ss);
}

void filterInplementationBlock::process() {
    auto solver = SolverManager::solver();
    if (!solver) return;

    std::vector<double> uvec = { inputValues[0] };
    solver->step(ss, uvec);

    std::vector<double> yvec = MatOp::matVecMul(ss.C, ss.x);
    double y = yvec[0] + ss.D[0][0] * inputValues[0];
    outputValues[0] = y;
}

void filterInplementationBlock::resetBefore() {
    for (int i = 0; i < outputValues.size(); i++) {
        outputValues[i] = 0.0;
        inputValues[i] = 0.0;
    }
    std::fill(ss.x.begin(), ss.x.end(), 0.0);
}



// -------------------------------------------------------------------------------------------------------------------------------------------
// kwadrat liczby
squaredBlock::squaredBlock(int id_) : Block(id_, 1, 1, false)
{
    size = ImVec2(200, 120);
}

void squaredBlock::process()
{
    outputValues[0] = std::pow(inputValues[0], 2);
}

void squaredBlock::drawContent()
{
    ImGui::Text("Squared");
}

// --------------------------------------------------------------------------------------------------------------------------------------------
// tranformacja pkt na jego wartosc funkji trygonometrycznej
TrigonometricFunctionBlock::TrigonometricFunctionBlock(int id_) : Block(id_, 1, 1, true) {
    size = ImVec2(200, 120);
}

void TrigonometricFunctionBlock::drawContent() {
    ImGui::Text("Trigonometric Block");
    ImGui::Text("Block Type: ");
    ImGui::SameLine();
    ImGui::Text(this->functionName.c_str());
    Block::drawContent();
}

void TrigonometricFunctionBlock::drawMenu() {
    static int current_item = 0;
    static const char* items[] = {"sin", "cos", "tan", "asin", "acos", "atan", "sinh", "cosh", "tanh"};

    if (ImGui::BeginCombo("Trigonometric Funcion", items[current_item], false)) {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
            bool is_selected = (current_item == n);
            if (ImGui::Selectable(items[n], is_selected)) {
                current_item = n;
                this->functionName = items[n];
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void TrigonometricFunctionBlock::process() {
    if (this->functionName == "sin")
        outputValues[0] = std::sin(inputValues[0]);
    else if (this->functionName == "cos")
        outputValues[0] = std::cos(inputValues[0]);
    else if (this->functionName == "tan")
        outputValues[0] = std::tan(inputValues[0]);
    else if (this->functionName == "asin")
        outputValues[0] = std::asin(inputValues[0]);
    else if (this->functionName == "acos")
        outputValues[0] = std::acos(inputValues[0]);
    else if (this->functionName == "atan")
        outputValues[0] = std::atan(inputValues[0]);
    else if (this->functionName == "sinh")
        outputValues[0] = std::sinh(inputValues[0]);
    else if (this->functionName == "cosh")
        outputValues[0] = std::cosh(inputValues[0]);
    else if (this->functionName == "tanh")
        outputValues[0] = std::tanh(inputValues[0]);
    else if (this->functionName == "asinh")
        outputValues[0] = std::asinh(inputValues[0]);
    else if (this->functionName == "acosh")
        outputValues[0] = std::acosh(inputValues[0]);
    else if (this->functionName == "atanh")
        outputValues[0] = std::atanh(inputValues[0]);
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// DeadZone blok
DeadZoneBlock::DeadZoneBlock(int id_) : Block(id_, 1, 1, true) {
    this->size = ImVec2(200, 120);
}

void DeadZoneBlock::process() {
    if (this->inputValues[0] > this->endDeadZone)
        this->outputValues[0] = this->inputValues[0] - this->endDeadZone;
    else if (this->inputValues[0] < -this->endDeadZone)
        this->outputValues[0] = this->inputValues[0] + this->endDeadZone;
    else
        this->outputValues[0] = 0.0;
}

void DeadZoneBlock::drawContent() {
    ImGui::Text("Dead Zone");
    Block::drawContent();
}

void DeadZoneBlock::drawMenu() {
    ImGui::InputDouble("Start Dead Zopne", &startDeadZone);
    ImGui::InputDouble("End Dead Zopne", &endDeadZone);
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// gain'a
GainBlock::GainBlock(int _id) : Block(_id, 1, 1, true) {
    size = ImVec2(200, 120);
}

void GainBlock::process() {
    outputValues[0] = inputValues[0] * multiplier;
    //std::cout<<"gain: "<<inputValues[0]<<" * "<<multiplier<<" = "<<outputValues[0]<<std::endl;
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



// -----------------------------------------------------------------------------------------------------------------------------------------------
// OR
logicORBlock::logicORBlock(int id_): Block(id_, 2, 1, true) {
    size = ImVec2(200, 120);
}

void logicORBlock::process() {
    for (int i = 0; i < numInputs; i++) {
        // to trzeba zmainic i ustwaic zgodnie z cmos
        if (inputValues[i] > 2.3) {
            outputValues[0] = 5;
            break;
        }
        outputValues[0] = 0;
    }
}


void logicORBlock::drawContent() {
    ImGui::Text("Logick OR");
    Block::drawContent();
}

void logicORBlock::drawMenu() {
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        if (numInputs < 2)
            numInputs = 2;
        else {
           // std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
            inputValues.resize(numInputs);
        }
    }
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// AND
logicANDBlock::logicANDBlock(int id_) : Block(id_, 2, 1, true) {
    size = ImVec2(200, 120);
}

void logicANDBlock::process() {
    for (int i = 0; i < numInputs; i++) {
        if (inputValues[i] < 2.3) {
            outputValues[0] = 0;
            break;
        }
        outputValues[0] = 5;
    }
}

void logicANDBlock::drawContent() {
    ImGui::Text("Logick AND");
    Block::drawContent();
}

void logicANDBlock::drawMenu() {
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        if (numInputs < 2)
            numInputs = 2;
        else {
           // std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
            inputValues.resize(numInputs);
        }
    }
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// NOT
logicNOTBlock::logicNOTBlock(int id_) : Block(id_, 1, 1, false) {
    size = ImVec2(200, 120);
}

void logicNOTBlock::process() {
    outputValues[0] = inputValues[0] < 2.3 ? 5 : 0;
}

void logicNOTBlock::drawContent() {
    ImGui::Text("Logic NOT");
    Block::drawContent();
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// NOR
logicNORBlock::logicNORBlock(int id_) : Block(id_, 2, 1, true) {
    size = ImVec2(200, 120);
}

void logicNORBlock::process() {
    for (int i = 0; i < numInputs; i++) {
        if (inputValues[i] > 2.3)
            outputValues[0] = 0;
        else
            outputValues[0] = 5;
    }
}

void logicNORBlock::drawContent() {
    ImGui::Text("Logic NOR");
    Block::drawContent();
}

void logicNORBlock::drawMenu() {
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        if (numInputs < 2)
            numInputs = 2;
        else {
            //std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
            inputValues.resize(numInputs);
        }
    }
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// python block
pythonBlock::pythonBlock(int _id) : Block(_id, 1, 1, true) {
    size = ImVec2(200, 120);
}

// tu trzeba dodac ten bytcode ale to poniej jak sie zainstaluje pybinda
void pythonBlock::process() {
    outputValues[0] = inputValues[0];
}

void pythonBlock::drawContent() {
    ImGui::Text("Python Block");
    Block::drawContent();
}

void pythonBlock::drawMenu() {
    // TODO: tu jescze ma sie zmainac wewntrzna funjja ze wzgledu an inplentacje
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        inputValues.resize(numInputs);
    }
    ImGui::SameLine();
    if (ImGui::InputInt("Number of outputs", &numOutputs)) {
        outputValues.resize(numOutputs);
    }

    ImGui::InputTextMultiline("Python Code", this->pythonCode, IM_ARRAYSIZE(this->pythonCode), ImVec2(400, 240));
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// cpp block
cppBlock::cppBlock(int _id) : Block(_id, 1, 1, true) {
    size = ImVec2(200, 120);
}

// tu trzeba dodac ten bytcode ale to poniej jak sie zainstaluje pybinda
void cppBlock::process() {
    outputValues[0] = inputValues[0];
}

void cppBlock::drawContent() {
    ImGui::Text("Python Block");
    Block::drawContent();
}

void cppBlock::drawMenu() {
    // TODO: tu jescze ma sie zmienic wewnetrzna funkcja ze wzgledu an implementacje
    if (ImGui::InputInt("Number of inputs", &numInputs)) {
        inputValues.resize(numInputs);
    }
    ImGui::SameLine();
    if (ImGui::InputInt("Number of outputs", &numOutputs)) {
        outputValues.resize(numOutputs);
    }

    ImGui::InputTextMultiline("Cpp Code", this->cppCode, IM_ARRAYSIZE(this->cppCode), ImVec2(400, 240));
}



// ----------------------------------------------------------------------------------------------------------------------------------------------
// data sending
DataSenderBlock::DataSenderBlock(int _id) : Block(_id, 1, 0, true), isInitialized(false), pipeName(R"(\\.\pipe\simulink_data)"), bufferSize(1000) {
    // rezerwacja miejsca dla bufora danych
    data.reserve(bufferSize);
    // instancja managera
    dataManager = DataChannelManager::getInstance();
}

DataSenderBlock::~DataSenderBlock() {
    if (dataManager && dataManager->isConnected())
        dataManager->close();
}

void DataSenderBlock::process() {
    if (!isInitialized) {
        // TODO: dodać pobranie wartości simTime i dt
        simTime = Model::simTime;
        dt = Model::timeStep;

        // inicjalizacja połączenia przy pierwszym uruchomieniu
        if (dataManager->initialize(pipeName)) {
            isInitialized = true;
            std::cout << "[DataSender " << id << "] Connected to Python receiver" << std::endl;
        }
        else {
            std::cerr << "[DataSender " << id << "] Failed to connect to Python receiver" << std::endl;
            return;
        }
    }

    data.push_back(inputValues[0]);
}

void DataSenderBlock::drawContent() {
    ImGui::Text("Data Sender");
    ImGui::Separator();

    if (isInitialized && dataManager->isConnected())
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
    else
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
}

void DataSenderBlock::drawMenu() {
    if (ImGui::BeginPopup(("BlockMenu" + std::to_string(id)).c_str())) {
        ImGui::Text("Data Sender Settings");
        ImGui::Separator();

        // ustawienia pipe'a
        char pipeNameBuf[256];
        strcpy(pipeNameBuf, pipeName.c_str());
        if (ImGui::InputText("Pipe Name", pipeNameBuf, sizeof(pipeNameBuf))) {
            pipeName = pipeNameBuf;
            // jeśli zmieniono nazwę, rozłącz i pozwól na ponowne połączenie
            if (isInitialized) {
                dataManager->close();
                isInitialized = false;
            }
        }
        ImGui::EndPopup();
    }
}

void DataSenderBlock::resetBefore() {
    data.clear();
}

void DataSenderBlock::resetAfter() {
    // wyślij dane do Pythona
    if (dataManager && dataManager->isConnected()) {
        if (!dataManager->sendData(data, dt, simTime)) {
            std::cerr << "[DataSender " << id << "] Failed to send data" << std::endl;
            // spróbuj ponownie połączyć przy następnym wywołaniu
            isInitialized = false;
        }
    }

    // // nie wiem czy będzie potrzebne, ale - wysyłanie pustego pakietu jako reset
    // if (isInitialized && dataManager->isConnected()) {
    //     std::vector<float> emptyData;
    //     // -1 jako sygnał resetu
    //     dataManager->sendData(emptyData, dt, -1.0f);
    // }
}

