//
// Created by patryk on 07.09.25.
//

#ifndef BASESOLVER_H
#define BASESOLVER_H

#include <vector>
#include "../matrix_operation/MatrixOperations.h"



class ISolverMethod {
public:
    virtual ~ISolverMethod() = default;
    virtual std::vector<double> step(MatOp::StateSpace& ss, const std::vector<double>& u, double dt)  = 0;

    template <class Archive>
    void serialize(Archive&) {
        // nic do zapisania w bazie
    }
};


#endif //BASESOLVER_H
