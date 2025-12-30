//
// Created by patryk on 07.09.25.
//
#pragma once

#include "../matrix_operation/MatrixOperations.h"
#include "ISolverMethod.h"



class Solver {
public:
    Solver(double dt, std::shared_ptr<ISolverMethod> method);

    // krok solvera dla danego układu
    std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u) const;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(dt));
        ar(CEREAL_NVP(method));
    }

private:
    double dt;
    std::shared_ptr<ISolverMethod> method;
};
