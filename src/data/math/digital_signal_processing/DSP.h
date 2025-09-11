//
// Created by patryk on 11.09.25.
//

#ifndef DSP_H
#define DSP_H

#include <vector>
#include <numbers>
#include <complex>

using cd = std::complex<double>;

namespace dsp{
    // funkje odpowiedzialne za transfomate fouriera
    void fft(std::vector<cd>& x, bool reverse);
    std::vector<cd> convolve(std::vector<cd> const& a, std::vector<cd> const& b);
    std::vector<cd> bluestein(std::vector<cd> const& x);

    // filtre
}

#endif //DSP_H
