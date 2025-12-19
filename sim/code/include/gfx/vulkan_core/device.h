#pragma once

#include "gfx/window.h"
#include "types.h"

namespace vulkan_core {
struct VulkanCore {
  vk::raii::Context context{};
  vk::raii::Instance instance{nullptr};

  // TODO: Add raii::DebugUtilsMessengerEXT later
  vk::raii::SurfaceKHR surface{nullptr};
  vk::raii::PhysicalDevice physicalDevice{nullptr};
  uint32_t graphicsQueueFamilyIndex{};
  vk::raii::Device device{nullptr};
  vk::raii::Queue graphicsQueue{nullptr};

  // CommandPool
  vk::raii::CommandPool commandPool{nullptr};
  std::unique_ptr<vk::raii::DescriptorPool> descriptor_pool{nullptr};
  std::array<Frame, IN_FLIGHT_FRAME_COUNT> frames{}; // Not inited

  vk::raii::Pipeline gfx_pipeline{nullptr};
  vk::raii::RenderPass render_pass{nullptr};
  // Uniform buffersn
  vk::raii::DescriptorSetLayout descriptor_set_layout{nullptr};
  vk::raii::PipelineLayout pipeline_layout{nullptr};
  vk::raii::Buffer vertex_buffer{nullptr};
  vk::raii::DeviceMemory vertex_buffer_memory{nullptr};
  void *vertex_buffer_mapped = nullptr;

  vk::raii::SwapchainKHR swapchain{nullptr}; // Not inited
  std::vector<vk::Image> swapchainImages{};  // Not inited
  std::vector<vk::raii::ImageView> swapchain_image_views{};
  std::vector<vk::raii::Framebuffer> swapchain_frame_buffers{};

  vk::Extent2D swapchainExtent{}; // Not inited
  vk::Format swapchainImageFormat{vk::Format::eB8G8R8A8Srgb};

  uint32_t currentSwapchainImageIndex{}; // Not inited
  uint32_t frameIndex{0};
  size_t particle_count{0};

  void clean_up();
};
error::CResult<VulkanCore> create_vulkan_core(const char *appName,
                                              window::MyWindow &window);
} // namespace vulkan_core
