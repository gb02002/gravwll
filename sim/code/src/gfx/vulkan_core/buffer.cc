#include "gfx/vulkan_core/buffer.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

namespace vulkan_core {
uint32_t find_memory_type(VulkanCore &core, uint32_t type_filter,
                          vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties mem_properties =
      core.physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
    if ((type_filter & (1 << i)) &&
        (mem_properties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type");
}

void create_buffer(VulkanCore &core, vk::DeviceSize size,
                   vk::BufferUsageFlags usage,
                   vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer,
                   vk::raii::DeviceMemory &buffer_memory) {
  vk::BufferCreateInfo buffer_info{};
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = vk::SharingMode::eExclusive;

  buffer = vk::raii::Buffer(core.device, buffer_info);

  vk::MemoryRequirements mem_requirements = buffer.getMemoryRequirements();

  vk::MemoryAllocateInfo alloc_info;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex =
      find_memory_type(core, mem_requirements.memoryTypeBits, properties);

  buffer_memory = vk::raii::DeviceMemory(core.device, alloc_info);
  buffer.bindMemory(*buffer_memory, 0);
}

void copy_to_buffer(VulkanCore &core, vk::raii::DeviceMemory &dstMemory,
                    const void *data, vk::DeviceSize size) {
  vk::MemoryMapInfo mapInfo{};
  mapInfo.memory = *dstMemory;
  mapInfo.offset = 0;
  mapInfo.size = size;
  mapInfo.flags = vk::MemoryMapFlags();

  void *mappedData = core.device.mapMemory2(mapInfo);
  memcpy(mappedData, data, static_cast<size_t>(size));

  vk::MemoryUnmapInfo unmapInfo{};
  unmapInfo.memory = *dstMemory;
  core.device.unmapMemory2(unmapInfo);
}

} // namespace vulkan_core
