#pragma once
#include "gfx/core/camera.h"
#include <cstddef>
#include <vector>

namespace gfx::renderer {

struct SceneParticle {
  glm::vec3 position;
  float mass;
  uint64_t visual_id;
};

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
