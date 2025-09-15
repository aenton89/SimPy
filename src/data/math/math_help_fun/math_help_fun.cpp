//
// Created by patryk on 12.09.25.
//

#include "math_help_fun.h"


// Funkja do mznorzneia wilomainaow
// potem trzba przerobic na tamplate
std::vector<std::complex<double>> math::expandPolynomial(const std::vector<std::complex<double>>& roots) {
    std::vector<std::complex<double>> coeffs = {1.0}; // startujemy od wielomianu 1

    for (const auto& root : roots) {
        std::vector<std::complex<double>> new_coeffs(coeffs.size() + 1, 0.0);

        for (size_t i = 0; i < coeffs.size(); i++) {
            new_coeffs[i]     += coeffs[i];        // x^i * x
            new_coeffs[i + 1] += -coeffs[i] * root; // x^i * (-root)
        }

        coeffs = new_coeffs;
    }
    return coeffs;
}