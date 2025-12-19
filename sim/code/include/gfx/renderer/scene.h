#pragma once
#include "gfx/core/camera.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gfx::renderer {

struct SceneParticle {
  glm::vec3 position;
  float mass;
  uint64_t visual_id;
};

// WARN: duplicate from types.h
constexpr uint64_t make_visual_id(uint8_t cat, uint8_t subtype, uint8_t shader,
                                  uint8_t texture, uint8_t lod,
                                  uint16_t sim_mode, uint16_t flags = 0) {
  return (uint64_t(cat) & 0xF) | ((uint64_t(subtype) & 0xF) << 4) |
         ((uint64_t(shader) & 0xFF) << 8) | ((uint64_t(texture) & 0xFF) << 16) |
         ((uint64_t(lod) & 0xFF) << 24) |
         ((uint64_t(sim_mode) & 0xFFFF) << 32) |
         ((uint64_t(flags) & 0xFFFF) << 48);
}

class Scene {
public:
  Scene();

  core::Camera &get_camera() { return camera_; }
  const core::Camera &get_camera() const { return camera_; }
  void set_particles(const std::vector<SceneParticle> &particles);

  const std::vector<SceneParticle> &get_particles() const {
    return particles_;
  };

  void update(float delta_time);

  size_t get_particles_count() const { return particles_.size(); }

private:
  core::Camera camera_;
  std::vector<SceneParticle> particles_;
};
} // namespace gfx::renderer
