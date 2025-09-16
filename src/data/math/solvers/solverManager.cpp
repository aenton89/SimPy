//
// Created by tajbe on 16.09.2025.
//

#include "solverManager.h"



std::shared_ptr<Solver>& SolverManager::solver() {
    static std::shared_ptr<Solver> instance = nullptr;
    return instance;
}

void SolverManager::initSolver(double dt, std::shared_ptr<ISolverMethod> method) {
    solver() = std::make_shared<Solver>(dt, method);
}


void SolverManager::clearSolver() {
    solver().reset();
}