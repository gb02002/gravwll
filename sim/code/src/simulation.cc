#include "simulation.h"
#include "ctx/simulation_config.h"
#include "engine/engine.h"
#include "gfx/gfx.h"
#include <chrono>
#include <iostream>

Simulation::Simulation(SimulationConfig &config)
    : ctx(config), PE(ctx.physics(), ctx.state(), ctx.storage(), ctx.data()),
      gfx(ctx.gfx(), ctx.state(), *PE.tree.get()) {
  std::cout << "Simulation initialized!\n";
};

void Simulation::initialization() {
  std::cout << "Simulation::Init\n";
  PE.Init();
  if (!ctx.gfx().headless) {
    std::cout << "Gfx init called\n";
    gfx.init();
  }
  return;
};

// We exit main thread if headless
void Simulation::run() {
  std::cout << "Simulation::Run\n";
  auto check_stepping = std::chrono::milliseconds(20);
  ctx.state().set_state(STATE::RUN);

  if (!ctx.gfx().headless)
    gfx.run();
  while (ctx.state().is_running()) {
    std::this_thread::sleep_for(std::chrono::duration(check_stepping));
  }
  return;
};
