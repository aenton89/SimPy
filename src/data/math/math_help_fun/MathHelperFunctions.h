//
// Created by patryk on 12.09.25.
//

#ifndef MATH_HELP_FUN_H
#define MATH_HELP_FUN_H

#include <vector>
#include <complex>



using cd = std::complex<double>;

namespace math {
    std::vector<std::complex<double>> expandPolynomial(const std::vector<std::complex<double>>& roots);

    // liczneie wartosci wielomianu w pkt f(z) = val
    cd polyEval(const std::vector<double>& coeff, cd z);

    // metoda Durand-Kerner do znajdowania pierwiastkow w wielomianie
    std::vector<cd> polyRoots_DK(const std::vector<double>& coeffs, int maxIter = 200, double tol = 1e-12);
}

#endif //MATH_HELP_FUN_H
