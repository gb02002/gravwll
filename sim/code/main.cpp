#include "simulation.h"

int main(int argc, char **argv) {
  auto sim = Simulation(argc, argv);
  sim.Init();
  sim.Run();
  return 0;
}
