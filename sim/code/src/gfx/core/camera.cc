#include "gfx/core/camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace gfx::core {

Camera::Camera() { update_vectors_(); }

void Camera::set_perspective(float fov_degrees, float aspect_ratio,
                             float near_plane, float far_plane) {
  fov_ = fov_degrees;
  aspect_ratio_ = aspect_ratio;
  near_plane_ = near_plane;
  far_plane_ = far_plane;
  projection_matrix_ = glm::perspective(glm::radians(fov_), aspect_ratio_,
                                        near_plane_, far_plane_);
}

void Camera::set_orthographic(float left, float right, float bottom, float top,
                              float near_plane, float far_plane) {
  projection_matrix_ =
      glm::ortho(left, right, bottom, top, near_plane, far_plane);
}

void Camera::look_at(const glm::vec3 &eye, const glm::vec3 &center,
                     const glm::vec3 &up) {
  position_ = eye;
  front_ = glm::normalize(center - eye);
  world_up_ = up;
  update_vectors_();
  view_matrix_ = glm::lookAt(position_, position_ + front_, up_);
}

void Camera::set_position(const glm::vec3 &position) {
  position_ = position;
  update_vectors_();
}

void Camera::set_yaw(float yaw_degrees) {
  yaw_ = yaw_degrees;
  update_vectors_();
}

void Camera::set_pitch(float pitch_degrees) {
  pitch_ = pitch_degrees;
  pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);
  update_vectors_();
}

void Camera::set_fov(float fov_degrees) {
  fov_ = fov_degrees;
  if (fov_ < 1.0f)
    fov_ = 1.0f;
  if (fov_ > 120.0f)
    fov_ = 120.0f;
  projection_matrix_ = glm::perspective(glm::radians(fov_), aspect_ratio_,
                                        near_plane_, far_plane_);
}

void Camera::update_vectors_() {
  glm::vec3 new_front;
  new_front.x =
      (float)(cos(glm::radians(yaw_)) * (float)cos(glm::radians(pitch_)));
  new_front.y = (float)(sin(glm::radians(yaw_)) * cos(glm::radians(pitch_)));
  new_front.z = (float)sin(glm::radians(pitch_));

  front_ = glm::normalize(new_front);

  right_ = glm::normalize(glm::cross(front_, world_up_));
  up_ = glm::normalize(glm::cross(right_, front_));

  view_matrix_ = glm::lookAt(position_, position_ + front_, up_);
}
} // namespace gfx::core
