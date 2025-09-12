//
// Created by patryk on 12.09.25.
//

#include <vector>
#include <complex>

using cd = std::complex<double>;

#ifndef MATH_HELP_FUN_H
#define MATH_HELP_FUN_H

namespace math {
    std::vector<std::complex<double>> expandPolynomial(const std::vector<std::complex<double>>& roots);
}

#endif //MATH_HELP_FUN_H
