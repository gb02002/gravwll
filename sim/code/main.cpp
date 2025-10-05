#include "simulation.h"

int main(int argc, char **argv) {
  auto config = SimulationConfigBuilder()
                    .with_defaults()               // in the future must
                    .with_config_file()            // transfer all logic in
                    .with_command_line(argc, argv) // build(), as in in argv
                    .build(); // we can pass config filename or can just reverse
                              // order and mark setted values in cli

  auto sim = Simulation(config);
  sim.initialization();
  sim.run();
  return 0;
}
