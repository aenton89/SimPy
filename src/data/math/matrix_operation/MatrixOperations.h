//
// Created by patryk on 07.09.25.
//

#ifndef MATRIX_OP_H
#define MATRIX_OP_H

#include <vector>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>



namespace MatOp {
    std::vector<double> matVecMul(const std::vector<std::vector<double>>& M, const std::vector<double>& v);
    std::vector<double> vecAdd(const std::vector<double>& v1, const std::vector<double>& v2);
    std::vector<double> scalarVecMul(double s, const std::vector<double>& v);

    struct StateSpace {
        // n x n
        std::vector<std::vector<double>> A;
        // n x m
        std::vector<std::vector<double>> B;
        // p x n
        std::vector<std::vector<double>> C;
        // p x m
        std::vector<std::vector<double>> D;
        // aktualny stan n x 1
        std::vector<double> x;

        template<class Archive>
        void serialize(Archive& ar) {
            ar(CEREAL_NVP(A),
               CEREAL_NVP(B),
               CEREAL_NVP(C),
               CEREAL_NVP(D),
               CEREAL_NVP(x));
        }
    };

    // lekka inplemntacja macierzy w cpp
    class Matrix {
    public:
        Matrix() = delete;

        Matrix(std::size_t rows, std::size_t cols);

        Matrix(Matrix &&) noexcept = default;

        Matrix &operator=(Matrix &&) noexcept = default;

        Matrix &operator=(const Matrix &) = default;

        void setData(std::vector<double> data);

        unsigned int getRows() const;
        unsigned int getCols() const;
        double* getData();
        const double* getData() const;

        // do debugowania
        void PrintData() const;
    private:
        std::size_t rows;
        std::size_t cols;
        std::vector<double> data_;
    };

    Matrix CPUMatMul(const Matrix& A, const Matrix& B);
    Matrix CPUMatAdd(const Matrix& A , const Matrix& B);
    Matrix CPUMatMulScalar(const Matrix& A, double s);
}

#endif //MATRIX_OP_H
