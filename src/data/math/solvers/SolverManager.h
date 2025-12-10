//
// Created by patryk on 07.09.25.
//
#pragma once

#include "Solver.h"
// #include <memory>



class SolverManager {
public:
    static std::shared_ptr<Solver>& solver();

    static void initSolver(double dt, const std::shared_ptr<ISolverMethod>& method);
    static void clearSolver();
};
