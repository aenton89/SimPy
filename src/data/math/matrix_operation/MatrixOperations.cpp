//
// Created by patryk on 07.09.25.
//
#include <vector>
#include "MatrixOperations.h"


// stare funkje do operacji ma vectorach 2d 3d ...
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


// Nowe funkje ma klasie Matrix
MatOp::Matrix::Matrix(std::size_t rows, std::size_t cols) {
    this->rows = rows;
    this->cols = cols;
    this->data_.resize(rows * cols);
}

unsigned int MatOp::Matrix::getCols() const {
    return this->cols;
}

unsigned int MatOp::Matrix::getRows() const {
    return this->rows;
}

double* MatOp::Matrix::getData() {
    return data_.data();
}

const double* MatOp::Matrix::getData() const {
    return data_.data();
}


void MatOp::Matrix::setData(const std::vector<double> data) {
    this->data_ = data;
}

void MatOp::Matrix::PrintData() const
{
    for (int i = 0; i < this->getRows(); i++) {
        for (int j = 0; j < this->getCols(); j++) {
            std::cout << this->data_[i * this->getCols() + j] << " ";
        }
        std::cout << std::endl;
    }
}

MatOp::Matrix MatOp::CPUMatAdd(const Matrix& A, const Matrix& B) {
    if (A.getCols() != B.getCols() || A.getRows() != B.getRows())
        throw std::runtime_error("Different sizes");

    Matrix result(A.getRows(), A.getCols());
    std::vector<double> data(A.getRows() * A.getCols());

    const double* Ad = A.getData();
    const double* Bd = B.getData();

    for (std::size_t i = 0; i < A.getRows(); i++) {
        for (std::size_t j = 0; j < A.getCols(); j++) {
            std::size_t idx = i * A.getCols() + j;
            data[idx] = Ad[idx] + Bd[idx];
        }
    }
    result.setData(data);
    return result;
}

MatOp::Matrix MatOp::CPUMatMul(const Matrix& A, const Matrix& B) {
    size_t m = A.getRows();
    size_t n = A.getCols();
    size_t k = B.getCols();

    if (n != B.getRows())
        throw std::runtime_error("Matrix size mismatch in CPUMatMul");

    Matrix result(m, k);
    const double* Ad = A.getData();
    const double* Bd = B.getData();
    double* Rd = result.getData();

    const size_t BLOCK = 32;

    // Tiling
    for (size_t ii = 0; ii < m; ii += BLOCK) {
        for (size_t jj = 0; jj < k; jj += BLOCK) {
            for (size_t pp = 0; pp < n; pp += BLOCK) {
                size_t i_max = std::min(ii + BLOCK, m);
                size_t j_max = std::min(jj + BLOCK, k);
                size_t p_max = std::min(pp + BLOCK, n);

                // Sumowanie po blokach
                for (size_t i = ii; i < i_max; i++) {
                    for (size_t j = jj; j < j_max; j++) {
                        double sum = 0;
                        for (size_t p = pp; p < p_max; p++) {
                            sum += Ad[i*n + p] * Bd[p*k + j];
                        }
                        Rd[i*k + j] += sum;
                    }
                }
            }
        }
    }

    return result;
}

MatOp::Matrix MatOp::CPUMatMulScalar(const Matrix& A, double s) {
    MatOp::Matrix result(A.getRows(), A.getCols());
    std::vector<double> data(A.getRows() * A.getCols());
    const double* Ad = A.getData();

    for (int i = 0; i < A.getRows()*A.getCols(); i++) {
        data[i] = s * Ad[i];
    }

    result.setData(data);

    return result;
}











