//
// Created by tajbe on 16.09.2025.
//

#include "memory"
#include "solver.h"



Solver::Solver(double dt, std::shared_ptr<ISolverMethod> method) : dt(dt), method(std::move(method)) {}

// krok solvera dla danego układu
std::vector<double> Solver::step(MatOp::StateSpace& ss, const std::vector<double>& u) {
    ss.x = method->step(ss, u, dt);
    return ss.x;
}