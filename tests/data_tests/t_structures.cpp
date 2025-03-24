//
// Created by tajbe on 24.03.2025.
//

#include <iostream>
#include "../../src/data/structures.h"
#include "../../src/data/blocks.h"


// testowy przykład użycia, bo nie chce mi sie pisać porządnego testu
void exampleUsage() {
    Model model;

    // dodawanie bloków
    auto sum = model.addBlock<SumBlock>("Sum1");
    auto mult = model.addBlock<MultiplyBlock>("Mult1");
    auto integ = model.addBlock<IntegratorBlock>("Integrator1", 0.01);

    // tworzenie połączeń, w tym sprzężenia zwrotnego
    model.connect(sum, 0, mult, 0);
   	model.connect(sum, 0, mult, 1);
    model.connect(mult, 0, integ, 0);
    // o tu! sprzężenie zwrotne
    model.connect(integ, 0, sum, 1);

    // symulacja, bo nie ma bloczka który podaje sygnał, więc robie to ręcznie
    for (int i = 0; i < 10; i++) {
      	// stała wartość wejściowa narazie
        sum->setInput(0, 1.0);
        model.simulate();
        std::cout << "Step " << i << ": " << integ->getOutput(0) << std::endl;
    }
}


int main() {
    exampleUsage();
    return 0;
}