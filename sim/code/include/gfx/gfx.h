#pragma once

#include "ctx/ctx.h"
#include "ctx/simulation_state.h"
#include "ds/tree/octree.h"
#include "renderer/renderer.h"
#include "utils/namespaces/error_namespace.h"
#include "window.h"

namespace gfx {

class GfxEngine {
public:
  explicit GfxEngine(GfxCtx &g_ctx, SimulationState &s_state, AROctree &tree);

  error::Result<bool> init();
  void run();
  void clean_up();

private:
  void tick_(float delta_time);

  GfxCtx &g_ctx_;            // gfx context
  SimulationState &s_state_; // shared state
  AROctree &tree_;

  window::MyWindow window_;
  std::unique_ptr<renderer::Renderer> render_;
};
} // namespace gfx
