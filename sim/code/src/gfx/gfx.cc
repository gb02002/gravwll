#include "gfx/gfx.h"
#include "utils/namespaces/error_namespace.h"
#include <SDL3/SDL_events.h>
#include <iostream>
#include <utility>

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
  auto res = window.show_window();
  if (res.is_error()) {
    debug::debug_print("Can't open window: {}", res.err_msg);
    return;
  }
  while (s_state.is_running()) {
    // handle events
    if (render->handle_events())
      s_state.request_exit();
    render->render();
  }
  std::cout << "we exit GfxEngine::run()\n";
};
void GfxEngine::clean_up() {};
