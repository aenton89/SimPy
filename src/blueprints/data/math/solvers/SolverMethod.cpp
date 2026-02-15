//
// Created by patryk on 07.09.25.
//
#include "SolverMethod.h"



// metody klasy RK1
std::vector<double> RK1Method::step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) {
    auto Ax = MatOp::matVecMul(ss.A, ss.x);
    auto Bu = MatOp::matVecMul(ss.B, u);
    auto dx = MatOp::vecAdd(Ax, Bu);

    ss.x = MatOp::vecAdd(ss.x, MatOp::scalarVecMul(dt, dx));
    return ss.x;
}

// metody klasy RK4
std::vector<double> RK4Method::step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) {
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

// metody klasy RK8
std::vector<double> RK8Method::step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) {
    using namespace MatOp;

    // Etapy RK8 (Butcher tableau dla RK8)
    std::vector<double> k1 = vecAdd(matVecMul(ss.A, ss.x), matVecMul(ss.B, u));
    std::vector<double> k2 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt/2, k1))), matVecMul(ss.B, u));
    std::vector<double> k3 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt/2, k2))), matVecMul(ss.B, u));
    std::vector<double> k4 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt, k3))), matVecMul(ss.B, u));
    std::vector<double> k5 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt*(2.0/3.0), k4))), matVecMul(ss.B, u));
    std::vector<double> k6 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt*(7.0/8.0), k5))), matVecMul(ss.B, u));
    std::vector<double> k7 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt*(8.0/9.0), k6))), matVecMul(ss.B, u));
    std::vector<double> k8 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt, k7))), matVecMul(ss.B, u));
    std::vector<double> k9 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt, k8))), matVecMul(ss.B, u));
    std::vector<double> k10 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt, k9))), matVecMul(ss.B, u));
    std::vector<double> k11 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt, k10))), matVecMul(ss.B, u));
    std::vector<double> k12 = vecAdd(matVecMul(ss.A, vecAdd(ss.x, scalarVecMul(dt, k11))), matVecMul(ss.B, u));

    // Wagi dla RK8 (klasyczny przykład, 13 punktów)
    std::vector<double> increment = vecAdd(
        scalarVecMul(1.0/15.0, k1),
        vecAdd(
            scalarVecMul(0.0, k2),  // niektóre współczynniki są zero
            vecAdd(
                scalarVecMul(0.0, k3),
                vecAdd(
                    scalarVecMul(16.0/45.0, k4),
                    vecAdd(
                        scalarVecMul(2.0/15.0, k5),
                        vecAdd(
                            scalarVecMul(16.0/45.0, k6),
                            vecAdd(
                                scalarVecMul(1.0/15.0, k7),
                                vecAdd(
                                    scalarVecMul(0.0, k8),
                                    vecAdd(
                                        scalarVecMul(0.0, k9),
                                        vecAdd(
                                            scalarVecMul(0.0, k10),
                                            vecAdd(
                                                scalarVecMul(0.0, k11),
                                                k12
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        )
    );

    ss.x = vecAdd(ss.x, scalarVecMul(dt, increment));
    return ss.x;
}


// metody kalsy RK2
std::vector<double> RK2Method::step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt) {
    std::vector<double> k1 = MatOp::matVecMul(ss.A, ss.x);
    std::vector<double> x_mid = MatOp::vecAdd(ss.x, MatOp::scalarVecMul(dt/2, k1));
    std::vector<double> k2 = MatOp::vecAdd(MatOp::matVecMul(ss.A, x_mid), MatOp::matVecMul(ss.B, u));

    ss.x = MatOp::vecAdd(ss.x, MatOp::scalarVecMul(dt, k2));
    return ss.x;
}



// dla serializacji
REGISTER_SOLVER_TYPE(RK1Method);
REGISTER_SOLVER_TYPE(RK2Method);
REGISTER_SOLVER_TYPE(RK4Method);
REGISTER_SOLVER_TYPE(RK8Method);