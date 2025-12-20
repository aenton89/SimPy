//
// Created by tajbe on 24.03.2025.
//
#pragma once

#include <array>
#include <random>
#include <complex>
#include <filesystem>
#include <fstream>
#include <numbers>
#include <cereal/types/complex.hpp>
#include "BasicBlock.h"
#include "Model.h"
#include "../../data/data_sender/DataChannelManager.h"
#include "../../data/math/matrix_operation/MatrixOperations.h"
#include "../../data/math/digital_signal_processing/DSP.h"
#if defined(__unix__) || defined(__APPLE__)
    #include <paths.h>
#endif
// #include <cereal/archives/xml.hpp>
// #include <cereal/types/polymorphic.hpp>
// #include <cereal/types/base_class.hpp>
// #include <cereal/types/set.hpp>
// #include <cereal/types/array.hpp>
// #include <cereal/types/vector.hpp>

// pomocnicze do jednoczesnej rejestracji i bloczków i ich polimorfizmu
#define REGISTER_BLOCK_TYPE(T) CEREAL_REGISTER_TYPE(T); CEREAL_REGISTER_POLYMORPHIC_RELATION(Block, T)



// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek sumujący
class SumBlock : public BlockCloneable<SumBlock> {
    // bo vector<bool> nie zwraca zwykłego wskaźnika (chyba??)
    std::vector<char> negate_inputs = {0, 0};
public:
    // konstruktor dla cereal
    SumBlock() : BlockCloneable<SumBlock>(-1, 2, 1, false) {}
    explicit SumBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(negate_inputs));
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek mnożący
class MultiplyBlock : public BlockCloneable<MultiplyBlock> {
public:
    // konstruktor dla cereal
    MultiplyBlock() : BlockCloneable<MultiplyBlock>(-1, 2, 1, false) {}
    explicit MultiplyBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek całkujący
class IntegratorBlock : public BlockCloneable<IntegratorBlock> {
private:
    double initial_state;
    //double state;
    MatOp::StateSpace ss;

public:
    // konstruktor dla cereal
    IntegratorBlock() : BlockCloneable<IntegratorBlock>(-1, 2, 1, false) {}
    // default'owo dt = 0.01
    explicit IntegratorBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void resetBefore() override;
    void setState(double initialState);
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(initial_state),
           CEREAL_NVP(ss));
    }
};

// -----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek rozniczkujacy
class DifferentiatorBlock : public BlockCloneable<DifferentiatorBlock> {
private:
    double initial_state;
    double state;
public:
    // konstruktor dla cereal
    DifferentiatorBlock() : BlockCloneable<DifferentiatorBlock>(-1, 2, 1, false) {}
    // default'owo dt = 0.01
    explicit DifferentiatorBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void resetAfter() override;
    void resetBefore() override;
    void setState(double initialState);
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(initial_state),
           CEREAL_NVP(state));
    }
};

// --------------------------------------------------------------------------------------------------------------------------------------------
// transmitacja operatorowa
class TransferFuncionContinous : public BlockCloneable<TransferFuncionContinous> {
public:
    // licznik
    std::string num;
    // mianownik
    std::string denum;
    std::vector<double> state;

    std::vector<double> numerator;
    std::vector<double> denominator;

    // konstruktor dla cereal
    TransferFuncionContinous() : BlockCloneable<TransferFuncionContinous>(-1, 2, 1, false) {}
    explicit TransferFuncionContinous(int _id);

    void process() override;
    void drawMenu() override;
    void drawContent() override;
    void resetBefore() override;

    static std::vector<double> stringToVector(const std::string& s);

private:
    bool run_tf2ss;
    MatOp::StateSpace ss;

public:
    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(num),
           CEREAL_NVP(denum),
           CEREAL_NVP(state),
           CEREAL_NVP(numerator),
           CEREAL_NVP(denominator),
           CEREAL_NVP(run_tf2ss),
           CEREAL_NVP(ss));
    }
};

// -----------------------------------------------------------------------------------------------------------------------------------------
// regulator PID
class PID_regulator : public BlockCloneable<PID_regulator> {
private:
    MatOp::StateSpace ss;
    // wzmocnienie dla czesci proporcionalenej
    double Kp = 1.0;
    // wzmocnienie dla czesci calkujacej
    double Ki = 1.0;
    // wzmocnienie czesci ronziczkujacej
    double Kd = 1.0;
    // filtr na czesc rozniczujaca
    double tau = 1.0;

    double state = 0;
    int current_mode = 0;

public:
    // konstruktor dla cereal
    PID_regulator() : BlockCloneable<PID_regulator>(-1, 2, 1, false) {}
    explicit PID_regulator(int _id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(ss),
           CEREAL_NVP(Kp),
           CEREAL_NVP(Ki),
           CEREAL_NVP(Kd),
           CEREAL_NVP(tau),
           CEREAL_NVP(state),
           CEREAL_NVP(current_mode));
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------
// FFT
class FFTBlock : public BlockCloneable<FFTBlock> {
private:
    long windowSize = 128;
    std::vector<cd> input_buffor;
    std::vector<cd> output_buffor;
    int type_of_work = 0;
    int counter = 0;
public:
    FFTBlock() : BlockCloneable <FFTBlock>(-1, 1, 1, true) {};
    explicit FFTBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
};

// Work in progres. Trzeba dokonczysc dopir jak da sie tempole na to zeby mnzona bylo przesylac array i triger czy przyjowac dane
class STFT_block : public BlockCloneable<STFT_block> {
private:
    long windowSize = 128;
    int current_window_mode = 0;
    long overlap = 64;
    double fs = 1/Model::timeStep;
    long nextPow2 = 128;
    int current_return_type = 0;

    // vector do przechowywania okna
    std::vector<double> window_vector;
    // vector do przechowyania batcha
    std::vector<std::complex<double>> batch_vector;


public:
    // konstruktor dla cereal
    STFT_block() : BlockCloneable<STFT_block>(-1, 2, 1, false) {}
    explicit STFT_block(int _id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    static std::vector<double> generateWindowVector(int N, int idx);

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(windowSize),
           CEREAL_NVP(current_window_mode),
           CEREAL_NVP(overlap),
           CEREAL_NVP(fs),
           CEREAL_NVP(nextPow2),
           CEREAL_NVP(current_return_type),
           CEREAL_NVP(window_vector),
           CEREAL_NVP(batch_vector));
    }
};

// --------------------------------------------------------------------------------------------------------------------------------------
// projektowanie filtrow
class filterImplementationBlock : public BlockCloneable<filterImplementationBlock> {
private:
    int current_signal_type = 0;
    int current_pass_type = 0;
    int analog_filter_type = 0;
    int digital_filter_type = 0;

    MatOp::StateSpace ss;
    dsp::tf Tf;

    dsp::FilterDesigner filter_designer;

    // typowe parametry filtra
    int filter_order = 1;
    float ripple = 1;
    double lower_limit = 5;
    double higher_limit = 10;
    std::vector<double> range = {lower_limit * 2 * std::numbers::pi, higher_limit * 2 * std::numbers::pi};

    static void drawBodePlot(const dsp::Bode& bode);

public:
    // konstruktor dla cereal
    filterImplementationBlock() : BlockCloneable<filterImplementationBlock>(-1, 2, 1, false) {}
    explicit filterImplementationBlock(int id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(current_signal_type),
           CEREAL_NVP(current_pass_type),
           CEREAL_NVP(analog_filter_type),
           CEREAL_NVP(digital_filter_type),
           CEREAL_NVP(ss),
           // jeśli nie ma jeszcze serialize dla dsp::tf i FilterDesigner, TO WYKOMENTUJ PATRYK
           CEREAL_NVP(filter_order),
           CEREAL_NVP(ripple),
           CEREAL_NVP(lower_limit),
           CEREAL_NVP(higher_limit),
           CEREAL_NVP(range));
    }
};

// --------------------------------------------------------------------------------------------------------------------------------------
// filtr medianowy 1D
class medianFilter1DBlock : public BlockCloneable<medianFilter1DBlock> {
private:
    std::vector<double> window_vector;
    long window_size = 5;
public:
    // konstruktor dla cereal
    medianFilter1DBlock() : BlockCloneable<medianFilter1DBlock>(-1, 2, 1, false) {}
    explicit medianFilter1DBlock(int id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(window_vector),
           CEREAL_NVP(window_size));
    }
};

// --------------------------------------------------------------------------------------------------------------------------------------
// filtr sredni (dziwna nazwa) poprostu movavg
class meanFilter1DBlock : public BlockCloneable<meanFilter1DBlock> {
private:
    std::vector<double> window_vector;
    long window_size = 5;
public:
    // konstruktor dla cereal
    meanFilter1DBlock() : BlockCloneable<meanFilter1DBlock>(-1, 2, 1, false) {}
    explicit meanFilter1DBlock(int id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(window_vector),
           CEREAL_NVP(window_size));
    }
};


// ----------------------------------------------------------------------------------------------------------------------------------------
// blok kwadratu sygnalu wejsciwoego
class squaredBlock : public BlockCloneable<squaredBlock> {
public:
    // konstruktor dla cereal
    squaredBlock() : BlockCloneable<squaredBlock>(-1, 2, 1, false) {}
    explicit squaredBlock(int id_);
    void process() override;
    void drawContent() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------
// blok pierwiskownaia
class sqrtBlock : public BlockCloneable<sqrtBlock> {
public:
    // konstruktor dla cereal
    sqrtBlock() : BlockCloneable<sqrtBlock>(-1, 2, 1, false) {}
    explicit sqrtBlock(int id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    std::string mode = "absolut value";

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(mode));
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek input'u
class StepBlock : public BlockCloneable<StepBlock> {
private:
    double inputValue;
    double delay = 0;
    double currentTime;
public:
    // konstruktor dla cereal
    StepBlock() : BlockCloneable<StepBlock>(-1, 2, 1, false) {}
    explicit StepBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(inputValue),
           CEREAL_NVP(delay),
           CEREAL_NVP(currentTime));
    }
};

// ---------------------------------------------------------------------------------------------------------------------------------------------
// bloczek inputu w postaci Sinusa
class SinusInputBlock : public BlockCloneable<SinusInputBlock> {
private:
   double inputValue;
   double shiftPhase = 0;
   double frequency = 1;
   double amplitude = 1;
   double currentTime = 0;
public:
    // konstruktor dla cereal
    SinusInputBlock() : BlockCloneable<SinusInputBlock>(-1, 2, 1, false) {}
    explicit SinusInputBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(inputValue),
           CEREAL_NVP(shiftPhase),
           CEREAL_NVP(frequency),
           CEREAL_NVP(amplitude),
           CEREAL_NVP(currentTime));
    }
};

// ---------------------------------------------------------------------------------------------------------------------------------------------
// bloczek inputu w postac PWM
class PWMInputBlock : public BlockCloneable<PWMInputBlock> {
private:
    double currentTime;
    double inputValue;
    double delay;
    double frequency;
    double dutyCycle;
public:
    // konstruktor dla cereal
    PWMInputBlock() : BlockCloneable<PWMInputBlock>(-1, 2, 1, false) {}
    explicit PWMInputBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(currentTime),
           CEREAL_NVP(inputValue),
           CEREAL_NVP(delay),
           CEREAL_NVP(frequency),
           CEREAL_NVP(dutyCycle));
    }
};

// -------------------------------------------------------------------------------------------------------------------------------------------------
// TODO: bloczek inputu z pliku? idk patryk doprecyzuj
class SignalFromFileBlock : public BlockCloneable<SignalFromFileBlock> {
private:
    std::string filePath;
    // 1 to odczyt sample po samplu dla duzych plikow a 0 to dla maych zaczytujemy calosc; szybsze gdy wyslamu do ESP
    int current_read_mode = 0;
    std::vector<double> buffor;

    float upper_band = 1;
    float lower_band = 0;

    bool is_scal = false;

    size_t i = 0;

    std::vector<double> values;

    float readNextValue();

public:
    // konstruktor dla cereal
    SignalFromFileBlock() : BlockCloneable<SignalFromFileBlock>(-1, 2, 1, false) {}
    explicit SignalFromFileBlock(int _id_);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetAfter() override;

    // indeks 'i' pomijamy, bo to stan chwilowy odczytu
    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(filePath),
           CEREAL_NVP(current_read_mode),
           CEREAL_NVP(buffor),
           CEREAL_NVP(upper_band),
           CEREAL_NVP(lower_band),
           CEREAL_NVP(is_scal),
           CEREAL_NVP(values));
    }
};

// -------------------------------------------------------------------------------------------------------------------------------------------------
// bloczek inputu w postaci bialego szumu
class WhiteNoiseInputBlock : public BlockCloneable<WhiteNoiseInputBlock> {
private:
    double mean = 0;
    double std = 1;
    double seed = 0;

    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
public:
    // konstruktor dla cereal
    WhiteNoiseInputBlock() : BlockCloneable<WhiteNoiseInputBlock>(-1, 2, 1, false) {}
    explicit WhiteNoiseInputBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    // TODO: patryk zweryfikuj xd
    // ten if constexpr ALBO generator i distribution inicjalizujemy po odczycie w konstruktorze lub resetBefore()
    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(mean),
           CEREAL_NVP(std),
           CEREAL_NVP(seed));

        // reinitialize generators after loading
        if constexpr (Archive::is_loading::value) {
            generator.seed(seed);
            distribution = std::normal_distribution<double>(mean, std);
        }
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek print'a
class PrintBlock : public BlockCloneable<PrintBlock> {
public:
    // konstruktor dla cereal
    PrintBlock() : BlockCloneable<PrintBlock>(-1, 2, 1, false) {}
    explicit PrintBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// bloczek robiący wykres
class PlotBlock : public BlockCloneable<PlotBlock> {
    // offset do przesunięcia wykresu
    float x_limMax = 0;
    float x_limMin = 0;
    // wskaźnik do danych do wykresu
    std::vector<std::vector<double>> data;
    float y_limMax = 1.0f;
    float y_limMin = -1.0f;
public:
    // konstruktor dla cereal
    PlotBlock() : BlockCloneable<PlotBlock>(-1, 2, 1, false) {}
    explicit PlotBlock(int _id);
    void process() override;
    void drawContent() override;
    void resetBefore() override;
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(x_limMax),
           CEREAL_NVP(x_limMin),
           CEREAL_NVP(data),
           CEREAL_NVP(y_limMax),
           CEREAL_NVP(y_limMin));
    }
};

// -------------------------------------------------------------------------------------------------------------------------------------------------
//bloczek wykresu XY
class PlotXYBlock : public BlockCloneable<PlotXYBlock> {
private:
    float x_limMax = 0;
    float x_limMin = 0;
    std::vector<std::array<double, 1000>> data;
    float y_limMax = 1.0f;
    float y_limMin = -1.0f;

    int sampleIndex = 0;
public:
    // konstruktor dla cereal
    PlotXYBlock() : BlockCloneable<PlotXYBlock>(-1, 2, 1, false) {}
    explicit PlotXYBlock(int _id);
    void process() override;
    void drawContent() override;
    void resetBefore() override;
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(x_limMax),
           CEREAL_NVP(x_limMin),
           CEREAL_NVP(data),
           CEREAL_NVP(y_limMax),
           CEREAL_NVP(y_limMin),
           CEREAL_NVP(sampleIndex));
    }
};

// -------------------------------------------------------------------------------------------------------------------------------------------------
// bloczke wykresu typu heatmap
class PlotHeatmapBlock : public BlockCloneable<PlotHeatmapBlock> {
private:
    std::vector<double> data;
    // liczba wierszy w kolumnie fft
    size_t num_row = 1;
public:
    // konstruktor dla cereal
    PlotHeatmapBlock() : BlockCloneable<PlotHeatmapBlock>(-1, 2, 1, false) {}
    explicit PlotHeatmapBlock(int _id);
    void process() override;
    void drawContent() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(data),
           CEREAL_NVP(num_row));
    }
};

// -------------------------------------------------------------------------------------------------------------------------------------------------
// blok saturacji
class SaturationBlock : public BlockCloneable<SaturationBlock> {
private:
    double upperLimit = 1.0f;
    double lowerLimit = -1.0f;
public:
    // konstruktor dla cereal
    SaturationBlock() : BlockCloneable<SaturationBlock>(-1, 2, 1, false) {}
    explicit SaturationBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(upperLimit),
           CEREAL_NVP(lowerLimit));
    }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// blok deadzona
class DeadZoneBlock : public BlockCloneable<DeadZoneBlock> {
private:
    double startDeadZone = -1.f;
    double endDeadZone = 1.f;
public:
    // konstruktor dla cereal
    DeadZoneBlock() : BlockCloneable<DeadZoneBlock>(-1, 2, 1, false) {}
    explicit DeadZoneBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(startDeadZone),
           CEREAL_NVP(endDeadZone));
    }
};

// zamina pkt na wartosc funkjo trygonometrycznej pkt
class TrigonometricFunctionBlock : public BlockCloneable<TrigonometricFunctionBlock> {
private:
    std::string functionName = "sin";
public:
    // konstruktor dla cereal
    TrigonometricFunctionBlock() : BlockCloneable<TrigonometricFunctionBlock>(-1, 2, 1, false) {}
    explicit TrigonometricFunctionBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(functionName));
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// jak MultiplyBlock, ale z ustawianym mnożnikiem
class GainBlock: public BlockCloneable<GainBlock> {
    float multiplier = 1.0f;
public:
    // konstruktor dla cereal
    GainBlock() : BlockCloneable<GainBlock>(-1, 2, 1, false) {}
    explicit GainBlock(int _id);
    void process() override;
    // TODO: GUI
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(multiplier));
    }
};

// ----------------------------------------------------------------------------------------------------------------------------------------------
// data sending - POKI CO WYSYLANIE NASTEPUJE W RESETAFTER(), ale to trzeba bedzie przeniesc
class DataSenderBlock : public BlockCloneable<DataSenderBlock> {
private:
    std::vector<double> data;
    double dt;
    double simTime;
    DataChannelManager* dataManager;
    bool isInitialized;
    std::string pipeName;
    int bufferSize;
public:
    // konstruktor dla cereal
    DataSenderBlock() : BlockCloneable<DataSenderBlock>(-1, 2, 1, false) {}
    explicit DataSenderBlock(int _id);
    ~DataSenderBlock() override;
    void process() override;
    void drawContent() override;
    void drawMenu() override;
    void resetBefore() override;
    void resetAfter() override;

    // TODO:  czy to jest okej? dataManager -> inicjalizujemy po wczytaniu
    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this),
           CEREAL_NVP(data),
           CEREAL_NVP(dt),
           CEREAL_NVP(simTime),
           CEREAL_NVP(isInitialized),
           CEREAL_NVP(pipeName),
           CEREAL_NVP(bufferSize));

        // reinitialize dataManager after loading
        if constexpr (Archive::is_loading::value) {
            dataManager = DataChannelManager::getInstance();
        }
    }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// bloki zwazane z inpelntacja kodu pythona i cpp w symualaci, rozwazam uzycie pybinda
class pythonBlock : public BlockCloneable<pythonBlock> {
public:
    // konstruktor dla cereal
    pythonBlock() : BlockCloneable<pythonBlock>(-1, 2, 1, false) {}
    explicit pythonBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
private:
    char pythonCode[512] = "def add(a, b): \n"
                           "   num = a + b \n"
                           "   return num \n";
public:
    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));

        // zamieniamy char[512] na std::string do serializacji
        std::string codeStr(pythonCode);
        ar(CEREAL_NVP(codeStr));

        // przy wczytywaniu kopiujemy z powrotem do tablicy
        if constexpr (Archive::is_loading::value) {
            std::strncpy(pythonCode, codeStr.c_str(), sizeof(pythonCode));
            // gwarancja null-terminacji
            pythonCode[sizeof(pythonCode) - 1] = '\0';
        }
    }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// cpp tu
class cppBlock : public BlockCloneable<cppBlock> {
public:
    // konstruktor dla cereal
    cppBlock() : BlockCloneable<cppBlock>(-1, 2, 1, false) {}
    explicit cppBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;
private:
    char cppCode[512] = "int add(int a, int b) { \n"
                        "   int num = a + b; \n"
                        "   return num; }; \n";
public:
    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));

        // konwersja tablicy char na std::string
        std::string codeStr(cppCode);
        ar(CEREAL_NVP(codeStr));

        // przy wczytywaniu kopiujemy z powrotem do tablicy
        if constexpr (Archive::is_loading::value) {
            std::strncpy(cppCode, codeStr.c_str(), sizeof(cppCode));
            // gwarancja null-terminacji
            cppCode[sizeof(cppCode) - 1] = '\0';
        }
    }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// OR block
class logicORBlock : public BlockCloneable<logicORBlock> {
public:
    // konstruktor dla cereal
    logicORBlock() : BlockCloneable<logicORBlock>(-1, 2, 1, false) {}
    explicit logicORBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));
    }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// AND block
class logicANDBlock : public BlockCloneable<logicANDBlock> {
public:
    // konstruktor dla cereal
    logicANDBlock() : BlockCloneable<logicANDBlock>(-1, 2, 1, false) {}
    explicit logicANDBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));
    }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// NOT block
class logicNOTBlock : public BlockCloneable<logicNOTBlock> {
public:
    // konstruktor dla cereal
    logicNOTBlock() : BlockCloneable<logicNOTBlock>(-1, 2, 1, false) {}
    explicit logicNOTBlock(int _id);
    void process() override;
    void drawContent() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));
    }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------
// NOR block
class logicNORBlock : public BlockCloneable<logicNORBlock> {
public:
    // konstruktor dla cereal
    logicNORBlock() : BlockCloneable<logicNORBlock>(-1, 2, 1, false) {}
    explicit logicNORBlock(int _id);
    void process() override;
    void drawContent() override;
    void drawMenu() override;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(cereal::base_class<Block>(this));
    }
};

#ifdef __linux__
    // ------------------------------------------------------------------------------------------------------------------------------------------------
    // ESP - output (blok ktory odbiera dane z esp)
    class ESPoutBlock : public BlockCloneable<ESPoutBlock> {
    public:
        ESPoutBlock() : BlockCloneable<ESPoutBlock>(-1, 0, 1, false) {} // w przyszloci zorbic zeby odbeiral wiecje niz jedno wysjcie
        explicit ESPoutBlock(int _id);
        void process() override;
        void drawContent() override;
        void drawMenu() override;

        template<class Archive>
        void serialize(Archive& ar) {
            ar(cereal::base_class<Block>(this));
        }
    };

    // ------------------------------------------------------------------------------------------------------------------------------------------------
    // ESP input block (blok ktory wrzuca pkt do generownaia po przez dac esp)
    class ESPinBlock : public BlockCloneable<ESPinBlock> {
    public:
        ESPinBlock() : BlockCloneable<ESPinBlock>(-1, 1, 0, false) {} // w prszysloci zrobic tak zeby odbieral wiecej niz jedno wejscie
        ESPinBlock(int _id);
        ~ESPinBlock();
        void process() override;
        void drawContent() override;
        void drawMenu() override;

        void connect();

        template<class Archive>
        void serialize(Archive& ar) {
            ar(cereal::base_class<Block>(this));
        }
    private:
        int fd;               // deskryptor portu
        bool connected;       // flaga połączenia

        int selectedPort = 0;
    };
#endif
