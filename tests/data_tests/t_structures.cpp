//
// Created by tajbe on 24.03.2025.
//

#include <iostream>
#include <vector>
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


std::vector<float> testPatryk(){
	Model model;
  	std::vector<float> output;

  	auto sum = model.addBlock<SumBlock>("Sum1");
  	auto integ = model.addBlock<IntegratorBlock>("Integrator1", 0.01);
  	auto mult = model.addBlock<MultiplyBlock>("Mult1");

  	model.connect(sum, 0, integ, 0);
  	model.connect(integ, 0, mult, 0);
  	model.connect(mult, 0, sum, 1);

  	for (int i = 0; i < 1000; i++){
      	sum->setInput(0, 1.0);
      	mult->setInput(1, -1.0);

      	model.simulate();
      	std::cout << "Step " << i << ": " << integ->getOutput(0) << std::endl;
      	output.push_back(integ->getOutput(0));
  	}

  	return output;
}


int main() {
    std::cout << std::endl << "Example test" << std::endl;
    std::cout << std::endl << "__________________________________________________\n" << std::endl;
    exampleUsage();

    std::cout << "Running patryk test" << std::endl;
    std::cout << std::endl << "__________________________________________________\n" << std::endl;
    std::vector<float> wynik = testPatryk();
    for(auto i : wynik){
      std::cout << i << "; ";
    }

    return 0;
}