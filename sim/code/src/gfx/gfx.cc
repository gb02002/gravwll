#include "gfx/gfx.h"
#include <iostream>

GfxEngine::GfxEngine(GfxCtx &g_ctx, SimulationState &s_state, AROctree &tree)
    : g_ctx(g_ctx), s_state(s_state), tree(tree) {};

void GfxEngine::init() {
  auto w_ = window.init();
  if (w_.is_error()) {
    debug::debug_print("Cound not init window: {}", w_.err_msg);
    return;
  }

  auto renderer_ = render::Renderer(window);
  render = std::move(renderer_);
};

void GfxEngine::tick() {};

void GfxEngine::run() {
  uint32_t total_frames = 0;

  if (auto res = window.show_window(); res.is_error()) {
    debug::debug_print("Can't open window: {}", res.err_msg);
    return;
  }

  while (s_state.is_running()) {
    total_frames++;

    if (window.handle_events())
      s_state.request_exit();

    render->render();
  }

  std::cout << "Exited GfxEngine::run(). Frame counter = " << total_frames
            << std::endl;
};

void GfxEngine::clean_up() {};
