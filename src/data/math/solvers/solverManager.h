//
// Created by patryk on 07.09.25.
//

#ifndef SOLVERMANAGER_H
#define SOLVERMANAGER_H

#include <memory>
#include "solver.h"



class SolverManager {
public:
    static std::shared_ptr<Solver>& solver();

    static void initSolver(double dt, std::shared_ptr<ISolverMethod> method);
    static void clearSolver();
};


#endif //SOLVERMANAGER_H
