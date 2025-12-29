#pragma once

#include "../vulkan_core/device.h"
#include "base_renderer.h"
#include "gfx/core/input.h"
#include "gfx/renderer/camera_controller.h"
#include "gfx/window.h"
#include "scene.h"
#include "utils/namespaces/error_namespace.h"
#include <cstddef>

namespace gfx::renderer {

class ParticleRenderer : public BaseRenderer {
public:
  ParticleRenderer(window::MyWindow &window);
  ~ParticleRenderer();

  error::Result<bool> init() override;
  error::Result<bool> render_frame() override;
  error::Result<bool> update(float delta_time) override;

  core::InputManager &get_input_manager() override { return input_manager_; }

  Scene &get_scene() { return scene_; }
  CameraController &get_camera_controller() { return camera_controller_; }

  error::Result<bool> update_uniform_buffer(const core::Camera &camera,
                                            float time, float point_size = 4.0f,
                                            float zoom_level = 1.0f,
                                            float brightness = 1.0f);

  void set_point_size(float size) { point_size_ = size; }
  float get_point_size() const { return point_size_; }

  void set_brightness(float brightness) { brightness_ = brightness; }
  float get_brightness() const { return brightness_; }

private:
  error::Result<bool> update_uniform_buffers();

  error::Result<bool> update_vertex_buffer_from_scene();
  error::Result<bool> init_vertex_buffer(size_t initial_capacity);
  error::Result<bool>
  update_vertex_buffer(const std::vector<vulkan_core::Vertex> &vertices);
  error::Result<bool> acquire_swapchain_image(vulkan_core::Frame &frame,
                                              uint32_t &image_index);
  error::Result<bool> submit_commands(vulkan_core::Frame &frame);
  error::Result<bool> present_frame(vulkan_core::Frame &frame,
                                    uint32_t image_index);
  error::Result<bool> recreate_vertex_buffer(size_t new_capacity);

  window::MyWindow &window_;
  core::InputManager input_manager_;
  Scene scene_;
  CameraController camera_controller_;

  size_t vertex_buffer_capacity_{0};
  vulkan_core::VulkanCore vulkan_core_;

  float total_time_ = 0.0f;
  float delta_time_ = 0.016f;
  bool initialized_ = false;

  float point_size_ = 4.0f;
  float brightness_ = 1.0f;
  float zoom_level_ = 1.0f;
};
} // namespace gfx::renderer
