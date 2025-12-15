#pragma once
#include "gfx/window.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <cstdint>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
// Категории (должны совпадать с C++ кодом)
const uint CAT_STAR = 0;
const uint CAT_PLANET = 1;
const uint CAT_ASTEROID = 2;
const uint CAT_DARK_MATTER = 3;
const uint CAT_BLACK_HOLE = 4;
const uint CAT_NEBULA = 5;
const uint CAT_DEBRIS = 6;
const uint CAT_UNKNOWN = 15;

// Подтипы звезд
const uint SUBTYPE_STAR_RED_DWARF = 0;
const uint SUBTYPE_STAR_YELLOW = 1;
const uint SUBTYPE_STAR_BLUE_GIANT = 2;
const uint SUBTYPE_STAR_RED_GIANT = 3;
const uint SUBTYPE_STAR_WHITE_DWARF = 4;
const uint SUBTYPE_STAR_NEUTRON = 5;
const uint SUBTYPE_STAR_BLACK_HOLE = 6;

// Подтипы планет
const uint SUBTYPE_PLANET_TERRESTRIAL = 0;
const uint SUBTYPE_PLANET_GAS_GIANT = 1;
const uint SUBTYPE_PLANET_ICE_GIANT = 2;
const uint SUBTYPE_PLANET_DWARF = 3;
const uint SUBTYPE_PLANET_OCEAN = 4;
const uint SUBTYPE_PLANET_DESERT = 5;

// Подтипы астероидов
const uint SUBTYPE_ASTEROID_ROCKY = 0;
const uint SUBTYPE_ASTEROID_METALLIC = 1;
const uint SUBTYPE_ASTEROID_ICY = 2;
const uint SUBTYPE_ASTEROID_CARBONACEOUS = 3;
namespace vulkan_core {

struct ImageLayout {
  vk::ImageLayout imageLayout{};
  vk::PipelineStageFlags2 stageMask{};
  vk::AccessFlags2 accessMask{};
  uint32_t queueFamilyIndex{VK_QUEUE_FAMILY_IGNORED};
};

struct CameraUBO {
  glm::mat4 view;
  glm::mat4 projection;
  glm::vec4 camera_pos;
  float point_size;
  float time;
  float zoom_level;
  float brightness;

  CameraUBO(float t_time) {
    point_size = 4.0f;
    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);
    camera_pos = glm::vec4(0.0f, 0.0f, 10.0f, 1.0f);
    point_size = 4.0f;
    time = t_time;
    zoom_level = 1.0f;
    brightness = 1.0f;
  };
};

struct UniformBufferObject {
  alignas(16) glm::mat4 mvp;
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

constexpr uint32_t IN_FLIGHT_FRAME_COUNT{3};

struct Frame {
  vk::raii::CommandBuffer commandBuffer{nullptr};
  vk::raii::Semaphore imageAvailableSemaphore{nullptr};
  vk::raii::Semaphore renderFinishedSemaphore{nullptr};
  vk::raii::Fence fence{nullptr};

  vk::raii::Buffer uniform_buffer{nullptr};
  vk::raii::DeviceMemory uniform_buffer_memory{nullptr};
  void *uniform_buffer_mapped{nullptr};
  vk::raii::DescriptorSet descriptor_set{nullptr};
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
  std::unique_ptr<vk::raii::DescriptorPool> descriptor_pool{nullptr};
  std::array<Frame, IN_FLIGHT_FRAME_COUNT> frames{}; // Not inited

  vk::raii::Pipeline gfx_pipeline{nullptr};
  vk::raii::RenderPass render_pass{nullptr};
  // Uniform buffersn
  vk::raii::DescriptorSetLayout descriptor_set_layout{nullptr};
  vk::raii::PipelineLayout pipeline_layout{nullptr};
  vk::raii::Buffer vertex_buffer{nullptr};
  vk::raii::DeviceMemory vertex_buffer_memory{nullptr};

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

error::Result<bool> create_vertex_buffer(VulkanCore &core);
error::CResult<VulkanCore> create_vulkan_core(const char *appName,
                                              window::MyWindow &window);
void init_frames(VulkanCore &core);
error::Result<bool> init_swapchain(VulkanCore &core, window::MyWindow &window);
SwapChainSupportDetails query_swapchain_support(VulkanCore &core);

error::Result<bool> create_graphics_pipeline(VulkanCore &);
error::Result<bool> create_render_pass(VulkanCore &);
error::Result<bool> record_frame_command_buffer(VulkanCore &core, Frame &frame,
                                                uint32_t imageIndex);
error::Result<bool> record_command_buffers(VulkanCore &core);
void record_command_buffer(VulkanCore &core, vk::CommandBuffer commandBuffer,
                           uint32_t imageIndex);
// error::Result<bool> create_command_buffers(VulkanCore &core);
error::Result<bool> create_image_views(VulkanCore &core);
error::Result<bool> create_framebuffers(VulkanCore &core);

error::Result<bool> create_descriptor_pool_and_sets(VulkanCore &core);
error::Result<bool> create_uniform_buffers(VulkanCore &core);
static std::vector<char> load_shader(const std::string &filename);

}; // namespace vulkan_core
