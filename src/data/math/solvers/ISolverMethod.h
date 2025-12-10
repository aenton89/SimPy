//
// Created by patryk on 07.09.25.
//
#pragma once

#include "../matrix_operation/MatrixOperations.h"
// #include <vector>



class ISolverMethod {
public:
    virtual ~ISolverMethod() = default;
    virtual std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt)  = 0;

    template <class Archive>
    void serialize(Archive&) {
        // nic do zapisania w bazie
    }
};
