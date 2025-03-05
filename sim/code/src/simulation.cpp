#include "simulation.h"
#include "engine/engine.h"
#include "gfx/gfx.h"
#include <iostream>
#include <thread>

Simulation::Simulation(int argc, char **argv)
    : cfx(argc, argv), PE(cfx), gfx(cfx, *PE.tree.get()) {
  std::cout << "Simulation initialized!\n";
};

void Simulation::Init() {
  std::cout << "Simulation::Init\n";
  PE.Init();
  gfx.Init();
  gfx.Tick();
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2000ms);
  return;
};
void Simulation::Run() {
  std::cout << "Simulation::Run\n";

  return;
}
