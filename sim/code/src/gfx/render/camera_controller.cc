#include "gfx/renderer/camera_controller.h"
#include "gfx/core/camera.h"
#include "gfx/core/input.h"
#include "utils/namespaces/error_namespace.h"
#include <exception>
#include <string>

namespace gfx::renderer {

CameraController::CameraController(core::Camera &camera,
                                   core::InputManager &input)
    : camera_(camera), input_(input) {
  debug::debug_print("CameraController inited");
}

error::Result<bool> CameraController::update(float delta_time) {
  if (!active_) {
    return error::Result<bool>::success(true);
  }

  try {
    const auto &input_state = input_.get_state();

    if (input_state.mouse.lmb && !input_.is_mouse_captured()) {
      input_.set_mouse_capture(true);
      first_mouse_ = true;
    }

    if (is_key_pressed(input_state, SDL_SCANCODE_F1)) {
      input_.set_mouse_capture(!input_.is_mouse_captured());
      first_mouse_ = true;
    }

    // 2. Выход по Escape (отпускаем захват или выходим)
    if (is_key_pressed(input_state, SDL_SCANCODE_ESCAPE)) {
      if (input_.is_mouse_captured()) {
        input_.set_mouse_capture(false);
        first_mouse_ = true;
      }
      // Можно добавить логику для выхода из приложения, если нужно
    }

    if (is_key_pressed(input_state, SDL_SCANCODE_R)) {
      camera_.set_position(glm::vec3(0.0f, 0.0f, 15.0f));
      camera_.look_at(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                      glm::vec3(0.0f, 0.0f, 1.0f));
      camera_.set_yaw(-90.0f);
      camera_.set_pitch(0.0f);
      std::cout << "Camera reset to initial position" << std::endl;
    }

    process_keyboard_(delta_time);
    process_mouse_();
    process_mouse_wheel_();

    return error::Result<bool>::success(true);
  } catch (const std::exception &e) {
    return error::Result<bool>::error(
        -1,
        std::string(std::string("CameraController update failed: ") + e.what())
            .c_str());
  }
}

void CameraController::process_keyboard_(float delta_time) {
  const auto &input_state = input_.get_state();

  float velocity = movement_speed_ * delta_time;

  if (is_key_held(input_state, SDL_SCANCODE_LSHIFT)) {
    velocity *= 3.0f;
  }

  if (is_key_held(input_state, SDL_SCANCODE_W)) {
    camera_.set_position(camera_.get_position() +
                         camera_.get_front() * velocity);
  }
  if (is_key_held(input_state, SDL_SCANCODE_S)) {
    camera_.set_position(camera_.get_position() -
                         camera_.get_front() * velocity);
  }

  if (is_key_held(input_state, SDL_SCANCODE_A)) {
    camera_.set_position(camera_.get_position() -
                         camera_.get_right() * velocity);
  }
  if (is_key_held(input_state, SDL_SCANCODE_D)) {
    camera_.set_position(camera_.get_position() +
                         camera_.get_right() * velocity);
  }

  if (is_key_held(input_state, SDL_SCANCODE_E)) {
    camera_.set_position(camera_.get_position() + camera_.get_up() * velocity);
  }
  if (is_key_held(input_state, SDL_SCANCODE_Q)) {
    camera_.set_position(camera_.get_position() - camera_.get_up() * velocity);
  }
}

void CameraController::process_mouse_() {
  if (!input_.is_mouse_captured()) {
    return;
  }

  const auto &input_state = input_.get_state();

  if (first_mouse_) {
    last_mouse_x_ = input_state.mouse.mouse_position.x;
    last_mouse_y_ = input_state.mouse.mouse_position.y;
    first_mouse_ = false;
  }

  float xoffset = input_state.mouse.delta.x * mouse_sensitivity_;
  float yoffset = input_state.mouse.delta.y * mouse_sensitivity_;

  camera_.set_yaw(camera_.get_yaw() + xoffset);
  camera_.set_pitch(camera_.get_pitch() -
                    yoffset); // Обратный знак для инвертирования

  camera_.set_pitch(std::clamp(camera_.get_pitch(), -89.0f, 89.0f));
}

void CameraController::process_mouse_wheel_() {
  const auto &input_state = input_.get_state();

  if (input_state.mouse.wheel != 0.0f) {
    movement_speed_ += input_state.mouse.wheel * zoom_speed_ * 0.1f;
    movement_speed_ = std::max(0.1f, std::min(movement_speed_, 100.0f));

    if (input_state.mouse.wheel != 0.0f) {
      std::cout << "Movement speed: " << movement_speed_ << std::endl;
    }
  }
}

} // namespace gfx::renderer
