#pragma once

#include "gfx/renderer/base_renderer.h"
#include "particle_renderer.h"
#include "utils/namespaces/error_namespace.h"
#include <memory>
#include <vector>

namespace gfx::renderer {
class Renderer : public BaseRenderer {
public:
  Renderer(window::MyWindow &window);
  ~Renderer() = default;

  error::Result<bool> init() override;
  error::Result<bool> render_frame() override;
  error::Result<bool> update(float delta_time) override;

  core::InputManager &get_input_manager() override;
  void add_particle_renderer(std::unique_ptr<ParticleRenderer> renderer);

  ParticleRenderer *get_particle_renderer();

private:
  window::MyWindow &window_;
  std::vector<std::unique_ptr<BaseRenderer>> render_passes_;

  ParticleRenderer *particle_renderer_ = nullptr;

  float total_time_ = 0.0f;
  float delta_time_ = 0.016f;
};
}; // namespace gfx::renderer
