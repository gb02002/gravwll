#include "simulation.h"
#include "ctx/simulation_config.h"
#include "engine/engine.h"
#include "gfx/gfx.h"
#include <iostream>

Simulation::Simulation(SimulationConfig &config)
    : ctx(config), PE(ctx.physics(), ctx.state(), ctx.storage(), ctx.data()),
      gfx(ctx.gfx(), ctx.state(), *PE.tree.get())

{
  std::cout << "Simulation initialized!\n";
};

void Simulation::initialization() {
  std::cout << "Simulation::Init\n";
  // PE.Init();
  // gfx.Init();
  return;
};

void Simulation::run() {
  std::cout << "Simulation::Run\n";
  // ctx.state().set_state(STATE::RUN);

  // gfx.Run();
  return;
};
