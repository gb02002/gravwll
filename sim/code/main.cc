#include "simulation.h"
#include "tbb/tbb.h"
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_reduce.h>

int main(int argc, char **argv) {
  auto config = SimulationConfigBuilder()
                    .with_defaults()                 // in the future must
                    .with_config_file("config.conf") // transfer all logic in
                    .with_command_line(argc, argv)   // build(), as in in argv
                    .build(); // we can pass config filename or can just reverse
                              // order and mark setted values in cli

  auto sim = Simulation(config);
  sim.initialization();
  sim.run();
  std::cout << "we exit" << std::endl;
  return 0;
}
