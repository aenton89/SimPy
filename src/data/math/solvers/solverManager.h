//
// Created by patryk on 07.09.25.
//
#include <memory>
#include "math/solvers/solver.h"

#ifndef SOLVERMANAGER_H
#define SOLVERMANAGER_H

class SolverManager {
public:
    static std::shared_ptr<Solver>& solver() {
        static std::shared_ptr<Solver> instance = nullptr;
        return instance;
    }

    static void initSolver(double dt, std::shared_ptr<ISolverMethod> method) {
        solver() = std::make_shared<Solver>(dt, method);
    }

    static void clearSolver() {
        solver().reset();
    }
};


#endif //SOLVERMANAGER_H
