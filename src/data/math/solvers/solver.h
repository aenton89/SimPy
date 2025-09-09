//
// Created by patryk on 07.09.25.
//

#ifndef SOLVER_H
#define SOLVER_H

#include "math/matrix_operation/matrix_op.h"
#include "math/solvers/ISolverMethod.h"

class Solver {
public:
    Solver(double dt, std::shared_ptr<ISolverMethod> method) : dt(dt), method(std::move(method)) {}

    // krok solvera dla danego układu
    std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u) {
        ss.x = method->step(ss, u, dt);
        return ss.x;
    }

private:
    double dt;
    std::shared_ptr<ISolverMethod> method;
};


#endif //SOLVER_H
