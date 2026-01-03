#include "simulation.h"
#include "tbb/tbb.h"
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_reduce.h>

int main(int argc, char **argv) {
  int sum = oneapi::tbb::parallel_reduce(
      oneapi::tbb::blocked_range<int>(1, 101), 0,
      [](oneapi::tbb::blocked_range<int> const &r, int init) -> int {
        for (int v = r.begin(); v != r.end(); v++) {
          init += v;
        }
        return init;
      },
      [](int lhs, int rhs) -> int { return lhs + rhs; });

  std::cout << sum << std::endl;
  return 0;
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
