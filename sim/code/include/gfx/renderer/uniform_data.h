#pragma once

#include "../core/camera.h"
#include "glm/ext/matrix_float4x4.hpp"

namespace gfx::renderer {

struct RenderUniform {
  glm::mat4 view;
  glm::mat4 projection;
  glm::vec4 camera_pos;
  float point_size;
  float time;
  float zoom_level;
  float brightness;

  static RenderUniform from_camera(const class core::Camera &camera,
                                   float aspect_ratio, float time = 0.0f,
                                   float point_size = 4.0f,
                                   float zoom_level = 1.0f,
                                   float brightness = 1.0f);
};
} // namespace gfx::renderer
