#include "gfx/gfx.h"
#include "utils/namespaces/error_namespace.h"
#include <algorithm>
#include <chrono>
#include <iostream>

namespace gfx {
GfxEngine::GfxEngine(GfxCtx &g_ctx, SimulationState &s_state, AROctree &tree)
    : g_ctx_(g_ctx), s_state_(s_state), tree_(tree) {};

error::Result<bool> GfxEngine::init() {
  if (auto e = window_.init(); e.is_error()) {
    debug::debug_print("Could not init window: {}", e.err_msg);
    return e;
  }

  if (auto res = window_.show_window(); res.is_error()) {
    return res;
  }

  render_ = std::make_unique<renderer::Renderer>(window_);
  if (auto e = render_->init(); e.is_error())
    return e;

  return error::Result<bool>::success(true);
};

void GfxEngine::run() {
  uint32_t total_frames = 0;
  auto last_time = std::chrono::high_resolution_clock::now();

  while (s_state_.is_running()) {
    total_frames++;

    auto &input_manager = render_->get_input_manager();
    input_manager.precess_events();

    if (input_manager.get_state().quit_requested) {
      s_state_.request_exit();
      break;
    }

    auto current_time = std::chrono::high_resolution_clock::now();
    float delta_time =
        std::chrono::duration<float>(current_time - last_time).count();
    last_time = current_time;

    delta_time = std::min(delta_time, 0.1f);
    render_->update(delta_time);

    if (auto e = render_->render_frame(); e.is_error()) {
      debug::debug_print("Failed to render frame: {}", e.err_msg);
      break;
    }
  }

  std::cout << "Exited GfxEngine::run(). Frame counter = " << total_frames
            << std::endl;
};

// TODO: figure out if we need separate clean_up or raii|destructors can handle
// it
void GfxEngine::clean_up() { render_.reset(); };
} // namespace gfx
