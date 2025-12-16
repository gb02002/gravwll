#pragma once
#include "ctx/ctx.h"
#include "ctx/simulation_state.h"
#include "ds/tree/octree.h"
#include "gfx/renderer/renderer.h"

class GfxEngine {
public:
  explicit GfxEngine(GfxCtx &g_ctx, SimulationState &s_state, AROctree &tree);

  void init();
  void tick();
  void run();
  void clean_up();

private:
  GfxCtx &g_ctx;            // gfx context
  SimulationState &s_state; // shared state
  AROctree &tree;

  window::MyWindow window;
  std::unique_ptr<render::Renderer> render;
};
