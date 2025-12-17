#include "gfx/renderer/renderer.h"
#include "gfx/window.h"
#include "utils/namespaces/error_namespace.h"
#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace gfx::renderer {

Renderer::Renderer(window::MyWindow &window) : window_(window) {};

error::Result<bool> Renderer::init() {
  debug::debug_print("Initing Renderer");

  auto particle_renderer = std::make_unique<ParticleRenderer>(window_);
  if (auto e = particle_renderer->init(); e.is_error())
    return e;

  particle_renderer_ = particle_renderer.get();
  render_passes_.push_back(std::move(particle_renderer));

  debug::debug_print("Renderer init success");
  return error::Result<bool>::success(true);
}

error::Result<bool> Renderer::render_frame() {
  for (auto &render_pass : render_passes_) {
    if (auto e = render_pass->render_frame(); e.is_error())
      return e;
  }
  return error::Result<bool>::success(true);
}

error::Result<bool> Renderer::update(float delta_time) {
  delta_time_ = delta_time;
  total_time_ += delta_time;

  for (auto &render_pass : render_passes_) {
    if (auto e = render_pass->update(delta_time); e.is_error())
      return e;
  }
  return error::Result<bool>::success(true);
}

core::InputManager &Renderer::get_input_manager() {
  return particle_renderer_->get_input_manager();
}

void Renderer::add_particle_renderer(
    std::unique_ptr<ParticleRenderer> renderer) {
  particle_renderer_ = renderer.get();
  render_passes_.push_back(std::move(renderer));
}

ParticleRenderer *Renderer::get_particle_renderer() {
  return particle_renderer_;
}
} // namespace gfx::renderer
