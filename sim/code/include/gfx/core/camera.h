#pragma once
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

namespace gfx::core {

class Camera {
public:
  Camera();

  void set_perspective(float fov, float aspect, float near, float far);
  void set_orthographic(float left, float right, float bottom, float top,
                        float near_plane, float far_plane);

  void look_at(const glm::vec3 &eye, const glm::vec3 &center,
               const glm::vec3 &up);
  void set_position(const glm::vec3 &position);

  const glm::vec3 &get_position() const { return position_; };
  const glm::vec3 &get_front() const { return front_; };
  const glm::vec3 &get_up() const { return up_; };
  const glm::vec3 &get_right() const { return right_; };

  const glm::mat4 &get_view_matrix() const { return view_matrix_; };
  const glm::mat4 &get_projection_matrix() const { return projection_matrix_; };

  void set_yaw(float yaw_degrees);
  void set_pitch(float pitch_degrees);
  float get_yaw() const { return yaw_; };
  float get_pitch() const { return pitch_; };

  void set_fov(float fov_degrees);
  float get_fov() const { return fov_; };

private:
  void update_vectors_();

  glm::vec3 position_ = glm::vec3(0.0f);
  glm::vec3 front_ = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 up_ = glm::vec3(0.0f, 0.0f, 1.0f);
  glm::vec3 right_ = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 world_up_ = glm::vec3(0.0f, 0.0f, 1.0f);

  float yaw_ = -90.0f;
  float pitch_ = 0.0f;

  float fov_ = 60.0f;
  float aspect_ratio_ = 16.0f / 9.0f;
  float near_plane_ = 0.1f;
  float far_plane_ = 1000.0f;

  glm::mat4 view_matrix_ = glm::mat4(1.0f);
  glm::mat4 projection_matrix_ = glm::mat4(1.0f);
};
} // namespace gfx::core
