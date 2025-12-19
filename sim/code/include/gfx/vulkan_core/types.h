#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace vulkan_core {
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

struct VisualID {
  uint64_t bits;

  inline uint8_t category() const { return bits & 0xF; }
  inline uint8_t subtype() const { return (bits >> 4) & 0xF; }
  inline uint8_t shader() const { return (bits >> 8) & 0xFF; }
  inline uint8_t texture() const { return (bits >> 16) & 0xFF; }
  inline uint8_t lod() const { return (bits >> 24) & 0xFF; }
  inline uint16_t sim_mode() const { return (bits >> 32) & 0xFFFF; }
  inline uint16_t flags() const { return (bits >> 48) & 0xFFFF; }
};

constexpr VisualID make_visual_id(uint8_t cat, uint8_t subtype, uint8_t shader,
                                  uint8_t texture, uint8_t lod,
                                  uint16_t sim_mode, uint16_t flags = 0) {
  return {(uint64_t(cat) & 0xF) | ((uint64_t(subtype) & 0xF) << 4) |
          ((uint64_t(shader) & 0xFF) << 8) |
          ((uint64_t(texture) & 0xFF) << 16) | ((uint64_t(lod) & 0xFF) << 24) |
          ((uint64_t(sim_mode) & 0xFFFF) << 32) |
          ((uint64_t(flags) & 0xFFFF) << 48)};
}
struct Vertex {
  alignas(16) glm::vec3 position; // 16 bytes
  float mass;                     // 4 bytes
  uint32_t visual_id_low;         // 4 bytes - младшие 32 бита
  uint32_t visual_id_high;        // 4 bytes - старшие 32 бита
};

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

} // namespace vulkan_core
