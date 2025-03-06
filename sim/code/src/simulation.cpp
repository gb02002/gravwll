#include "simulation.h"
#include "engine/engine.h"
#include "gfx/gfx.h"
#include <iostream>

Simulation::Simulation(int argc, char **argv)
    : cfx(argc, argv), PE(cfx), gfx(cfx, *PE.tree.get()) {
  std::cout << "Simulation initialized!\n";
};

void Simulation::Init() {
  std::cout << "Simulation::Init\n";
  PE.Init();
  gfx.Init();
  return;
};

void Simulation::Run() {
  std::cout << "Simulation::Run\n";
  cfx.state = RUN;

  gfx.Run();
  return;
}
