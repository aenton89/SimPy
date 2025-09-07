//
// Created by patryk on 07.09.25.
//

#include <math/solvers/solverMethod.h>

std::vector<double> RK1Method::step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) {
    auto Ax = MatOp::matVecMul(ss.A, ss.x);
    auto Bu = MatOp::matVecMul(ss.B, u);
    auto dx = MatOp::vecAdd(Ax, Bu);
    ss.x = MatOp::vecAdd(ss.x, MatOp::scalarVecMul(dt, dx));
    return ss.x; // lub void, jeśli robisz update w ss.x bezpośrednio
}

std::vector<double> RK4Method::step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt)
{
    std::vector<double> k1 = MatOp::vecAdd(MatOp::matVecMul(ss.A, ss.x), MatOp::matVecMul(ss.B, u));
    std::vector<double> k2 = MatOp::vecAdd(MatOp::matVecMul(ss.A, MatOp::vecAdd(ss.x, MatOp::scalarVecMul(dt/2, k1))), MatOp::matVecMul(ss.B, u));
    std::vector<double> k3 = MatOp::vecAdd(MatOp::matVecMul(ss.A, MatOp::vecAdd(ss.x, MatOp::scalarVecMul(dt/2, k2))), MatOp::matVecMul(ss.B, u));
    std::vector<double> k4 = MatOp::vecAdd(MatOp::matVecMul(ss.A, MatOp::vecAdd(ss.x, MatOp::scalarVecMul(dt, k3))), MatOp::matVecMul(ss.B, u));

    // x_{k+1} = x_k + dt/6 * (k1 + 2*k2 + 2*k3 + k4)
    std::vector<double> increment = MatOp::vecAdd(
        k1,
        MatOp::vecAdd(
            MatOp::scalarVecMul(2.0, k2),
            MatOp::vecAdd(
                MatOp::scalarVecMul(2.0, k3),
                k4
            )
        )
    );

    ss.x = MatOp::vecAdd(ss.x, MatOp::scalarVecMul(dt/6.0, increment));
    return ss.x;
}





