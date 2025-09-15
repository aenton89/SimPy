//
// Created by patryk on 07.09.25.
//

#ifndef SOLVER_H
#define SOLVER_H

#include "../matrix_operation/matrix_op.h"
#include "ISolverMethod.h"



class Solver {
public:
    Solver(double dt, std::shared_ptr<ISolverMethod> method);

    // krok solvera dla danego układu
    std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u);

private:
    double dt;
    std::shared_ptr<ISolverMethod> method;
};


#endif //SOLVER_H
