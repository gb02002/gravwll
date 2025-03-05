#pragma once
#include "engine/engine.h"
#include "gfx/gfx.h"

class Simulation {
public:
  Simulation(int argc, char **argv);

  void Init();
  void Run();

private:
  Cfx cfx;
  PhysicsEngine PE;
  GfxEngine gfx;
};
