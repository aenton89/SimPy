//
// Created by tajbe on 24.03.2025.
//

#include "blocks.h"
#include <sstream>
#include <cmath>
#include <numbers>

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
    currentTime += this->timeStep;
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
    currentTime += this->timeStep;
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

    this->currentTime += this->timeStep;
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
    x_limMax = this->simTime;
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
            ImPlot::PlotLine(label.c_str(), data[i].data(), data[i].size(), this->timeStep);
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



// ---------------------------------------------------------------------------------------------------------------------------------------
// roznczkowanie
DifferentiatorBlock::DifferentiatorBlock(int _id) : Block(_id, 1, 1, true), initial_state(0.0) {}

void DifferentiatorBlock::process() {
    double derivative = (inputValues[0] - initial_state) / timeStep;
    initial_state = inputValues[0];
    outputValues[0] = derivative;

    std::cout << "differentiator: (" << inputValues[0] << " - " << initial_state << ") / " << timeStep << " = " << outputValues[0] << std::endl;
}

// TODO: GUI
void DifferentiatorBlock::drawContent() {
    ImGui::Text("Time step: ");
    ImGui::InputDouble("", &timeStep);
    ImGui::Text("Different: %f", outputValues[0]);

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


void printStateSpace(const MatOp::StateSpace& ss) {
    std::cout << "A:\n";
    for (const auto& row : ss.A) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "B:\n";
    for (const auto& row : ss.B) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "C:\n";
    for (const auto& row : ss.C) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "D:\n";
    for (const auto& row : ss.D) {
        for (double v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "x (stan początkowy): ";
    for (double v : ss.x) std::cout << v << " ";
    std::cout << "\n";
}

MatOp::StateSpace TransferFuncionContinous::tf2ss(std::vector<float> numerator, std::vector<float> denominator) {

    if (denominator.empty()) {
        throw std::invalid_argument("Mianownik nie może być pusty.");
    }

    // konwersja float -> double
    std::vector<double> num(numerator.begin(), numerator.end());
    std::vector<double> den(denominator.begin(), denominator.end());

    // usunięcie zer wiodących
    while (!num.empty() && std::fabs(num.front()) < 1e-12) num.erase(num.begin());
    while (!den.empty() && std::fabs(den.front()) < 1e-12) den.erase(den.begin());

    if (den.empty()) {
        throw std::invalid_argument("Mianownik po redukcji nie może być pusty.");
    }

    int deg_num = (int)num.size() - 1;
    int deg_den = (int)den.size() - 1;
    int n = deg_den;

    // normalizacja do wiodącego 1 w mianowniku
    double lead = den[0];
    for (auto &v : den) v /= lead;
    for (auto &v : num) v /= lead;

    // wyzerowanie wektora num do długości n+1
    std::vector<double> num_padded(n+1, 0.0);
    for (int i = 0; i < (int)num.size(); i++) {
        num_padded[n+1 - num.size() + i] = num[i];
    }

    std::vector<double> a(den.begin() + 1, den.end());

    double Dval = num_padded[0];

    std::vector<double> cvec(n, 0.0);
    for (int i = 0; i < n; i++) {
        cvec[i] = num_padded[i+1] - a[i] * Dval;
    }

    // macierz A (n x n)
    std::vector<std::vector<double>> A(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n-1; i++) {
        A[i][i+1] = 1.0;
    }
    for (int j = 0; j < n; j++) {
        A[n-1][j] = -a[n-1-j];
    }

    // macierz B (n x 1)
    std::vector<std::vector<double>> B(n, std::vector<double>(1, 0.0));
    B[n-1][0] = 1.0;

    // macierz C (1 x n)
    std::vector<std::vector<double>> C(1, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        C[0][i] = cvec[n-1-i];
    }

    // macierz D (1 x 1)
    std::vector<std::vector<double>> D(1, std::vector<double>(1, Dval));

    // wektor stanu x (n x 1), inicjalizacja zerami
    std::vector<double> x(n, 0.0);

    MatOp::StateSpace ss{A, B, C, D, x};

    printStateSpace(ss);
    return ss;
}


// zapsianie wetora do postaci wielomainu
std::string polyToString(std::vector<float>& coeffs) {
    std::string result;
    int n = coeffs.size();

    for (int i = 0; i < n; ++i) {
        int coeff = coeffs[i];
        int power = n - i - 1;

        if (coeff == 0)
            continue;

        if (!result.empty())
            result += " + ";

        if (power == 0) {
            result += std::to_string(coeff);
        }
        else if (power == 1) {
            if (coeff == 1)
                result += "s";
            else
                result += std::to_string(coeff) + "s";
        }
        else {
            if (coeff == 1)
                result += "s^" + std::to_string(power);
            else
                result += std::to_string(coeff) + "s^" + std::to_string(power);
        }
    }

    return result.empty() ? "0" : result;
}

void TransferFuncionContinous::drawContent() {
    ImGui::Text("Transfer Funcion Continous");
    ImGui::Separator();
    ImGui::Text(polyToString(this->numerator).c_str());
    ImGui::Text("-------------");
    ImGui::Text(polyToString(this->denominator).c_str());

    if (this->run_tf2ss) {
        this->numerator = stringToVector(num);
        this->denominator = stringToVector(denum);

        // if (this->numerator.size() < this->denominator.size())
        ss = TransferFuncionContinous::tf2ss(this->numerator, this->denominator);
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
    // 1. pobierz globalny solver
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
            std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
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
            std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
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
            std::cout<<"changed number of inputs: "<<numInputs<<std::endl;
            inputValues.resize(numInputs);
        }
    }
}

// ----------------------------------------------------------------------------------------------------------------------------------------------
// python block
pythonBlock::pythonBlock(int _id) : Block(_id, 1, 1, true)
{
    size = ImVec2(200, 120);
}

void pythonBlock::process()
{
    // tu trzeba dodab ten bytcode ale to poniej jak sie zainsluje pybinda
    outputValues[0] = inputValues[0];
}

void pythonBlock::drawContent()
{
    ImGui::Text("Python Block");
    Block::drawContent();
}

void pythonBlock::drawMenu()
{
    if (ImGui::InputInt("Number of inputs", &numInputs))
    {
        // TODO tu jescze ma sie zmainac wewntrzna funjja ze wzgledu an inplentacje
        inputValues.resize(numInputs);
    }
    ImGui::SameLine();
    if (ImGui::InputInt("Number of outputs", &numOutputs))
    {
        outputValues.resize(numOutputs);
    }

    ImGui::InputTextMultiline("Python Code", this->pythonCode, IM_ARRAYSIZE(this->pythonCode), ImVec2(400, 240));
}

// ----------------------------------------------------------------------------------------------------------------------------------------------
// cpp block
cppBlock::cppBlock(int _id) : Block(_id, 1, 1, true)
{
    size = ImVec2(200, 120);
}

void cppBlock::process()
{
    // tu trzeba dodab ten bytcode ale to poniej jak sie zainsluje pybinda
    outputValues[0] = inputValues[0];
}

void cppBlock::drawContent()
{
    ImGui::Text("Python Block");
    Block::drawContent();
}

void cppBlock::drawMenu()
{
    if (ImGui::InputInt("Number of inputs", &numInputs))
    {
        // TODO tu jescze ma sie zmainac wewntrzna funjja ze wzgledu an inplentacje
        inputValues.resize(numInputs);
    }
    ImGui::SameLine();
    if (ImGui::InputInt("Number of outputs", &numOutputs))
    {
        outputValues.resize(numOutputs);
    }

    ImGui::InputTextMultiline("Cpp Code", this->cppCode, IM_ARRAYSIZE(this->cppCode), ImVec2(400, 240));
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
