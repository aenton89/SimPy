//
// Created by patryk on 11.09.25.
//

#define LPF 0
#define HPF 1
#define BPF 2
#define BSF 3

#ifndef DSP_H
#define DSP_H

#include <vector>
#include <numbers>
#include <complex>
#include "math/matrix_operation/matrix_op.h"
#include <iostream>

using cd = std::complex<double>;

namespace dsp{
    // funkje odpowiedzialne za transfomate fouriera
    void fft(std::vector<cd>& x, bool reverse);
    std::vector<cd> convolve(std::vector<cd> const& a, std::vector<cd> const& b);
    std::vector<cd> bluestein(std::vector<cd> const& x);

    // transmitanja operatorowa
    MatOp::StateSpace tf2ss(std::vector<float> numerator, std::vector<float> denominator);

    struct tf {
        std::vector<cd> zeros; // wielomian licznika
        std::vector<cd> poles; // wielomian mianownika
        double gain;
    };

    // charaterystyki bodego
    struct Bode {
        std::vector<double> magnitude;
        std::vector<double> phase;
        std::vector<double> omega;
    };

    Bode bode_characteristic(const tf Tf);

    // filtry

    void printStateSpace(const MatOp::StateSpace& ss);

    tf butterworth(int order, int filter_type, const std::vector<double>& cutoff);
}

#endif //DSP_H
