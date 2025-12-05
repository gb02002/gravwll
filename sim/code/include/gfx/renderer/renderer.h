#pragma once
#include "core/bodies/particles.h"
#include "gfx/vulkan_core.h"
#include "utils/namespaces/error_namespace.h"
#include <vector>

namespace render {
class Renderer {
  vulkan_core::VulkanCore primitives;

  error::Result<bool> begin_frame(const vulkan_core::Frame &fr);

  void SubmitCommandBuffer(vulkan_core::Frame const &frame);
  void EndFrame(vulkan_core::Frame const &frame);

  std::vector<Particle> particles;

public:
  Renderer(window::MyWindow &window);
  error::Result<bool> render();
  error::Result<bool> render_frame();

private:
  void update_uniform_buffer(vulkan_core::Frame &frame);

  void record_draw_commands(vulkan_core::Frame &frame, uint32_t imageIndex);
  void submit_commands(const vulkan_core::Frame &frame);
  void present_frame(const vulkan_core::Frame &frame);
};
}; // namespace render
