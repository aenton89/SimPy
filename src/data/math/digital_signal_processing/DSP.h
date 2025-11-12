//
// Created by patryk on 11.09.25.
//
#ifndef DSP_H
#define DSP_H

#define LPF 0
#define HPF 1
#define BPF 2
#define BSF 3

#define BUTTERWORTH 0
#define CHEBYSHEV_I 1
#define CHEBYSHEV_II 2
#define BESSEL 3
#define ELLIPTICAL 4

#include <vector>
#include <numbers>
#include <complex>
#include <iostream>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include "../matrix_operation/MatrixOperations.h"

using cd = std::complex<double>;



namespace dsp{
    // funkje odpowiedzialne za transfomate fouriera
    void fft(std::vector<cd>& x, bool reverse);
    std::vector<cd> convolve(std::vector<cd> const& a, std::vector<cd> const& b);
    std::vector<cd> bluestein(std::vector<cd> const& x);

    // transmitanja operatorowa
    MatOp::StateSpace tf2ss(std::vector<double> numerator, std::vector<double> denominator);

    struct tf {
        // wielomian licznika
        std::vector<cd> zeros;
        // wielomian mianownika
        std::vector<cd> poles;
        double gain;

        template<class Archive>
        void serialize(Archive& ar) {
            ar(CEREAL_NVP(zeros),
               CEREAL_NVP(poles),
               CEREAL_NVP(gain));
        }
    };

    // charaterystyki bodego
    struct Bode {
        std::vector<double> magnitude;
        std::vector<double> phase;
        std::vector<double> omega;

        template <class Archive>
        void serialize(Archive& ar) {
            ar(CEREAL_NVP(magnitude),
               CEREAL_NVP(phase),
               CEREAL_NVP(omega));
        }
    };

    Bode bode_characteristic(const tf& Tf);


    // filtry
    class FilterDesigner {
    private:
        int order = 1;
        int filter_type = BUTTERWORTH;
        int filter_subtype = LPF;
        float ripple = 1.0;

        tf Tf;
        std::vector<double> cutoff = {5, 10};

    protected:
        tf butterworth_proto();
        tf chebyshev_i_proto();
        tf chebyshev_ii_proto();
        tf besel_proto();

        void apply_filter_subtype();

    public:
        FilterDesigner();

        void apply_setting(int order, int filter_type, int filter_subtype, float ripple, std::vector<double> cutoff);
        tf get_tf();

        template<class Archive>
        void serialize(Archive& ar) {
            ar(CEREAL_NVP(order),
               CEREAL_NVP(filter_type),
               CEREAL_NVP(filter_subtype),
               CEREAL_NVP(ripple),
               CEREAL_NVP(Tf),
               CEREAL_NVP(cutoff));
        }
    };



    void printStateSpace(const MatOp::StateSpace& ss);

    // filtr butherwortha
    // tf butterworth_proto(const int& order);
    // // filtr chebyszewicza 1 rodzaju
    // tf chebyshev_1_proto(const int& order);
    //
    // // uniwersalna metoda do zamiany LPF na inne
    // tf apply_filter_subtype(const int& filter_type, dsp::tf tf_zp, const std::vector<double>& cutoff);

    //tf butterworth(int order, int filter_type, const std::vector<double>& cutoff);
}

#endif //DSP_H
