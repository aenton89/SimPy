//
// Created by tajbe on 24.03.2025.
//

#ifndef BLOCKS_H
#define BLOCKS_H

#include "structures.h"
#include <implot.h>
#include <array>
#include <random>
#include "data_sender/data_channel_manager.h"
#include "math/matrix_operation/matrix_op.h"
#include <complex>



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
    void resetBefore() override;
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek mnożący
class MultiplyBlock : public Block {
public:
    MultiplyBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek całkujący
class IntegratorBlock : public Block {
private:
    double initial_state;
    //double state;
    MatOp::StateSpace ss;

public:
    // default'owo dt = 0.01
    IntegratorBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void resetAfter() override;
    void resetBefore() override;
    void setState(double initialState);
    void drawMenu() override;
};

// -----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek rozniczkujacy
class DifferentiatorBlock : public Block {
private:
    double initial_state;
    double state;
public:
    // default'owo dt = 0.01
    DifferentiatorBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void resetAfter() override;
    void resetBefore() override;
    void setState(double initialState);
    void drawMenu() override;
};

// --------------------------------------------------------------------------------------------------------------------------------------------
// transmitacja operatorowa
class TransferFuncionContinous : public Block{
public:
    std::string num;   // licznik
    std::string denum; // mianownik
    std::vector<double> state;

    std::vector<float> numerator;
    std::vector<float> denominator;

    TransferFuncionContinous(int _id);

    void process() override;
    void drawMenu() override;
    void drawContent() override;
    void resetBefore() override;
    std::vector<float> stringToVector(const std::string& s);
    MatOp::StateSpace tf2ss(std::vector<float> numerator, std::vector<float> denominator);
private:
    bool run_tf2ss;
    MatOp::StateSpace ss;
};

// -----------------------------------------------------------------------------------------------------------------------------------------
// regulator PID
class PID_regulator : public Block
{
private:
    MatOp::StateSpace ss;
    double Kp = 1.0; // wsmocniee dla czesci proporcionalenej
    double Ki = 1.0; // wzmocneinie dla czesci calkujacej
    double Kd = 1.0; // wzmocneinei czesci ronziczkujacej
    double tau = 1.0; // filtr na czesc rozniczujaca

    double state = 0;

    int current_mode = 0;

public:
    PID_regulator(int _id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
};

// ----------------------------------------------------------------------------------------------------------------------------------------
// FFT

class STFT_block : public Block  // Work in progres. Trzeba dokonczysc dopir jak da sie tempole na to zeby mnzona bylo przesylac array i triger czy przyjowac dane
{
private:
    long windowSize = 128;
    int current_window_mode = 0;
    long overlap = 64;
    double fs = 1/Model::timeStep;

    long nextPow2 = 128;

    int current_return_type = 0;

    std::vector<double> window_vector; // vector do przechowywania okna
    std::vector<std::complex<double>> batch_vector; // vector do przechowyania batcha


public:
    STFT_block(int _id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    std::vector<double> generateWindowVector(int N, int idx);
};

// --------------------------------------------------------------------------------------------------------------------------------------
// Projektowanie filtrow
class filterInplementationBlock : public Block {
private:

public:
    filterInplementationBlock(int id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
};


// ----------------------------------------------------------------------------------------------------------------------------------------
// blok kwadratu sygnalu wejsciwoego
class squaredBlock : public Block {
public:
    squaredBlock(int id_);
    void process() override;
    void drawContent() override;
};

// ----------------------------------------------------------------------------------------------------------------------------------------
// blok pierwiskownaia
class sqrtBlock : public Block {
public:
    sqrtBlock(int id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    std::string mode = "absolut value";
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek input'u
class StepBlock : public Block {
private:
    double inputValue;
    double delay = 0;
    double currentTime;
public:
    StepBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------
// bloczek inputu w postaci Sinusa
class SinusInputBlock : public Block {
private:
   double inputValue;
   double shiftPhase = 0;
   double frequency = 1;
   double amplitude = 1;
   double currentTime = 0;
public:
   SinusInputBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------
// bloczek inputu w postac PWM
class PWMInputBlock : public Block {
private:
    double currentTime;
    double inputValue;
    double delay;
    double frequency;
    double dutyCycle;
public:
    PWMInputBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
};

// -------------------------------------------------------------------------------------------------------------------------------------------------
// bloczek inputu w postaci bialego szumu
class WhiteNoiseInputBlock : public Block {
private:
    double mean = 0;
    double std = 1;
    double seed = 0;

    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
public:
    WhiteNoiseInputBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
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
    // offset do przesunięcia wykresu
    float x_limMax = 0;
    float x_limMin = 0;
    // wskaźnik do danych do wykresu
    std::vector<std::vector<float>> data;
    float y_limMax = 1.0f;
    float y_limMin = -1.0f;
public:
    PlotBlock(int _id);
    void process() override;
    void drawContent() override;
    void resetBefore() override;
    void drawMenu() override;
};

//bloczek wykresu XY
class PLotXYBlock : public Block {
private:
    float x_limMax = 0;
    float x_limMin = 0;
    std::vector<std::array<float, 1000>> data;
    float y_limMax = 1.0f;
    float y_limMin = -1.0f;

    int sampleIndex = 0;
public:
    PLotXYBlock(int _id);
    void process() override;
    void drawContent() override;
    void resetBefore() override;
    void drawMenu() override;
};

// bloczke wykresu typu heatmap
class PlotHeatmapBlock : public Block {
private:
    std::vector<float> data;
    size_t num_row = 1; // liczba wierszy w kolumnie fft
public:
    PlotHeatmapBlock(int _id);
    void process() override;
    void drawContent() override;
    void resetBefore() override;
    void drawMenu() override;
};

// -------------------------------------------------------------------------------------------------------------------------------------------------
// blok saturacji
class SaturationBlock : public Block {
private:
    double upperLimit = 1.0f;
    double lowerLimit = 1.0f;
public:
    SaturationBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// blok deadzona
class DeadZoneBlock : public Block {
private:
    double startDeadZone = -1.f;
    double endDeadZone = -1.f;
public:
    DeadZoneBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
};

// zamina pkt na wartosc funkjo trygonometrycznej pkt
class TrigonometricFunctionBlock : public Block {
private:
    std::string functionName = "sin";
public:
    TrigonometricFunctionBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
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
    void resetBefore() override;
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// data sending - POKI CO WYSYLANIE NASTEPUJE W RESETAFTER(), ale to trzeba bedzie przeniesc
class DataSenderBlock : public Block {
private:
    std::vector<float> data;
    double dt;
    double simTime;
    DataChannelManager* dataManager;
    bool isInitialized;
    std::string pipeName;
    int bufferSize;
public:
    DataSenderBlock(int _id);
    ~DataSenderBlock();
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
    void resetAfter() override;
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// bloki zwazane z inpelntacja kodu pythona i cpp w symualaci, rozwazam uzycie pybinda
class pythonBlock : public Block {
public:
    pythonBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
private:
    char pythonCode[512] = "def add(a, b): \n"
                           "   num = a + b \n"
                           "   return num \n";
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// cpp tu
class cppBlock : public Block {
public:
    cppBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
private:
    char cppCode[512] = "int add(int a, int b) { \n"
                        "   int num = a + b; \n"
                        "   return num; }; \n";
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// OR block
class logicORBlock : public Block {
public:
    logicORBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
};

class logicANDBlock : public Block {
public:
    logicANDBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// NOT block
class logicNOTBlock : public Block {
public:
    logicNOTBlock(int _id);
    void process() override;
    void drawContent() override;
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// NOR block
class logicNORBlock : public Block {
public:
    logicNORBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
};

#endif
