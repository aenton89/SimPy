//
// Created by patryk on 07.09.25.
//

#ifndef SOLVERMETHOD_H
#define SOLVERMETHOD_H

#include "math/solvers/ISolverMethod.h"

// Euler do przodu RK1
class RK1Method : public ISolverMethod {
public:
    std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) override;
};

// RK4
class RK4Method : public ISolverMethod
{
public:
    std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) override;
};

#endif //SOLVERMETHOD_H
