//
// Created by patryk on 07.09.25.
//

#include <vector>
#include "matrix_op.h"

// funkcja mnożenia macierzy przez wektor
std::vector<double> MatOp::matVecMul(const std::vector<std::vector<double>>& M, const std::vector<double>& v) {
    std::vector<double> result(M.size(), 0.0);
    for (size_t i = 0; i < M.size(); i++) {
        for (size_t j = 0; j < v.size(); j++) {
            result[i] += M[i][j] * v[j];
        }
    }
    return result;
}

// dodawanie dwóch wektorów
std::vector<double> MatOp::vecAdd(const std::vector<double>& v1, const std::vector<double>& v2) {
    std::vector<double> result(v1.size());
    for (size_t i = 0; i < v1.size(); i++) {
        result[i] = v1[i] + v2[i];
    }
    return result;
}

// mnożenie wektora przez skalar
std::vector<double> MatOp::scalarVecMul(double s, const std::vector<double>& v) {
    std::vector<double> result(v.size());
    for (size_t i = 0; i < v.size(); i++) {
        result[i] = s * v[i];
    }
    return result;
}
