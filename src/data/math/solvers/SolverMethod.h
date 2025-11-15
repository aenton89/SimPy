//
// Created by patryk on 07.09.25.
//

#ifndef SOLVERMETHOD_H
#define SOLVERMETHOD_H

#include "ISolverMethod.h"

#define REGISTER_SOLVER_TYPE(T) CEREAL_REGISTER_TYPE(T) CEREAL_REGISTER_POLYMORPHIC_RELATION(ISolverMethod, T)



// Euler do przodu RK1
class RK1Method : public ISolverMethod {
public:
    std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) override;

    template <class Archive>
    void serialize(Archive&) {}
};

// RK4
class RK4Method : public ISolverMethod {
public:
    std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) override;

    template <class Archive>
    void serialize(Archive&) {}
};

// RK2
class RK2Method : public ISolverMethod {
public:
    std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) override;

    template <class Archive>
    void serialize(Archive&) {}
};

#endif //SOLVERMETHOD_H
