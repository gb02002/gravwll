#pragma once
#include "gfx/vulkan_core.h"
#include "utils/namespaces/error_namespace.h"

namespace render {
class Renderer {
  vulkan_core::VulkanCore primitives;

  error::Result<bool> begin_frame(const vulkan_core::Frame &fr);

  void SubmitCommandBuffer(vulkan_core::Frame const &frame);
  void EndFrame(vulkan_core::Frame const &frame);

public:
  Renderer(window::MyWindow &window);
  error::Result<bool> render();
};
}; // namespace render
