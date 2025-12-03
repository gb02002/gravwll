#pragma once
#include "gfx/window.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

const std::vector<const char *> validation_layers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = true;

#endif // NDEBUG

namespace vulkan_core {

bool check_validation_layer_support();

struct ImageLayout {
  vk::ImageLayout imageLayout{};
  vk::PipelineStageFlags2 stageMask{};
  vk::AccessFlags2 accessMask{};
  uint32_t queueFamilyIndex{VK_QUEUE_FAMILY_IGNORED};
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

error::CResult<vk::raii::Device> get_device();
error::CResult<vk::raii::Instance> get_instance(const char *name);
error::CResult<vk::raii::SurfaceKHR> get_surface();
error::CResult<vk::raii::PhysicalDevices> get_physical_device();
error::Result<uint32_t> get_queue_famity_index();
error::CResult<vk::raii::Queue> get_queue();
error::CResult<vk::raii::SwapchainKHR> get_swapchain();

error::CResult<vk::raii::CommandPool> get_command_pool();

constexpr uint32_t IN_FLIGHT_FRAME_COUNT{3};

struct Frame {
  vk::raii::CommandBuffer commandBuffer{nullptr};
  vk::raii::Semaphore imageAvailableSemaphore{nullptr};
  vk::raii::Semaphore renderFinishedSemaphore{nullptr};
  vk::raii::Fence fence{nullptr};
};

struct VulkanCore {
  vk::raii::Context context{};
  vk::raii::Instance instance{nullptr};
  vk::raii::SurfaceKHR surface{nullptr};
  vk::raii::PhysicalDevice physicalDevice{nullptr};
  uint32_t graphicsQueueFamilyIndex{};
  vk::raii::Device device{nullptr};
  vk::raii::Queue graphicsQueue{nullptr};

  vk::raii::CommandPool commandPool{nullptr};
  std::array<Frame, IN_FLIGHT_FRAME_COUNT> frames{}; // Not inited

  VkPipeline gfx_pipeline;
  vk::RenderPass render_pass;
  vk::raii::SwapchainKHR swapchain{nullptr}; // Not inited
  std::vector<vk::Image> swapchainImages{};  // Not inited
  vk::Extent2D swapchainExtent{};            // Not inited
  vk::Format swapchainImageFormat{vk::Format::eB8G8R8A8Srgb};
  uint32_t currentSwapchainImageIndex{}; // Not inited
  uint32_t frameIndex{0};
};

error::CResult<VulkanCore> create_vulkan_core(const char *appName,
                                              window::MyWindow &window);
void init_frames(VulkanCore &core);
error::Result<bool> init_swapchain(VulkanCore &core, window::MyWindow &window);
SwapChainSupportDetails query_swapchain_support(VulkanCore &core);
error::Result<bool> create_graphics_pipeline(VulkanCore &);
error::Result<bool> create_render_pass(VulkanCore &);

static std::vector<char> load_shader(const std::string &filename);

}; // namespace vulkan_core
