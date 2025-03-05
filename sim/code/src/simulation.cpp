#include "simulation.h"
#include "engine/engine.h"
#include <iostream>

Simulation::Simulation(int argc, char **argv) : cfx(argc, argv), PE(cfx) {
  std::cout << "Simulation initialized!\n";
};

void Simulation::Init() {
  std::cout << "Simulation::Init\n";
  return;
};
void Simulation::Run() {
  std::cout << "Simulation::Run\n";
  return;
}
