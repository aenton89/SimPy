//
// Created by patryk on 12.09.25.
//

#include "math_help_fun.h"


// Funkja do mznorzneia wilomainaow
// potem trzba przerobic na tamplate
std::vector<std::complex<double>> math::expandPolynomial(const std::vector<std::complex<double>>& roots) {
    std::vector<std::complex<double>> coeffs = {cd(1.0, 0)}; // startujemy od wielomianu 1

    for (const auto& root : roots) {
        std::vector<std::complex<double>> new_coeffs(coeffs.size() + 1, cd(0.0, 0.0));

        for (size_t i = 0; i < coeffs.size(); i++) {
            new_coeffs[i]     += coeffs[i];        // x^i * x
            new_coeffs[i + 1] += -coeffs[i] * root; // x^i * (-root)
        }

        coeffs = new_coeffs;
    }
    return coeffs;
}

cd math::polyEval(const std::vector<double>& coeff, cd z) {
    cd result = cd(0.0, 0.0);
    for (int i = coeff.size() - 1; i >= 0; --i) {
        result = result * z + coeff[i];
    }
    return result;
}

std::vector<cd> math::polyRoots_DK(const std::vector<double>& coeffs, int maxIter, double tol) {
    int n = coeffs.size() - 1;
    std::vector<cd> roots(n);

    for (int i = 0; i < n; i++) {
        double angle = 2.0 * M_PI * i / n;
        roots[i] = cd(std::cos(angle), std::sin(angle));
    }

    for (int iter = 0; iter < maxIter; iter++) {
        bool done = true;
        std::vector<cd> new_roots = roots;

        for (int i = 0; i < n; i++) {
            cd denum = cd(1.0, 0.0);
            for (int j = 0; j < n; j++) {
                if (i != j) denum *= (roots[i] - roots[j]);
            }
            new_roots[i] = roots[i] - polyEval(coeffs, roots[i]) / denum;

            if (std::abs(new_roots[i] - roots[i]) > tol) {
                done = false;
            }
        }
        roots = new_roots;
        if (done) break;
    }

    return roots;
}



