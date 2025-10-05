#pragma once
#include "ctx/simulation_config.h"
#include "engine/engine.h"
#include "gfx/gfx.h"

class Simulation {
public:
  Simulation(SimulationConfig &config);

  void initialization();
  void run();
  // void CleanUp();

private:
  Ctx ctx;
  PhysicsEngine PE;
  GfxEngine gfx;
};
