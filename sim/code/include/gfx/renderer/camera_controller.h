#pragma once

#include "../core/camera.h"
#include "../core/input.h"
#include "utils/namespaces/error_namespace.h"

namespace gfx::renderer {

class CameraController {
public:
  CameraController(core::Camera &camera, core::InputManager &input);

  error::Result<bool> update(float delta_time);
  void set_movement_speed(float speed) { movement_speed_ = speed; };
  void set_zoom_speed(float speed) { zoom_speed_ = speed; };

  bool is_active() const { return active_; };
  void set_active(bool active) { active_ = active; };

private:
  void process_keyboard_(float delta_time);
  void process_mouse_(float delta_time);
  void process_mouse_wheel_(float delta_time);

  core::Camera &camera_;
  core::InputManager &input_;

  float movement_speed_ = 10.0f;
  float mouse_sensitivity_ = 0.1f;
  float zoom_speed_ = 2.0f;

  bool active_ = true;
  bool first_mouse_ = true;
  float last_mouse_x_ = 0.0f;
  float last_mouse_y_ = 0.0f;
};
} // namespace gfx::renderer
