//
// Created by patryk on 07.09.25.
//

#include <vector>

#ifndef MATRIX_OP_H
#define MATRIX_OP_H

namespace MatOp
{
    std::vector<double> matVecMul(const std::vector<std::vector<double>>& M, const std::vector<double>& v);
    std::vector<double> vecAdd(const std::vector<double>& v1, const std::vector<double>& v2);
    std::vector<double> scalarVecMul(double s, const std::vector<double>& v);

    struct StateSpace {
        std::vector<std::vector<double>> A; // n x n
        std::vector<std::vector<double>> B; // n x m
        std::vector<std::vector<double>> C; // p x n
        std::vector<std::vector<double>> D; // p x m
        std::vector<double> x;              // aktualny stan n x 1
    };
}

#endif //MATRIX_OP_H
