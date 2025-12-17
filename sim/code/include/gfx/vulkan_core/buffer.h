#pragma once

#include "device.h"
#include <cstdint>

namespace vulkan_core {

uint32_t find_memory_type(VulkanCore &core, uint32_t type_filter,
                          vk::MemoryPropertyFlags properties);
void create_buffer(VulkanCore &core, vk::DeviceSize size,
                   vk::BufferUsageFlags usage,
                   vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer,
                   vk::raii::DeviceMemory &buffer_memory);

void copy_to_buffer(VulkanCore &core, vk::raii::DeviceMemory &dstMemory,
                    const void *data, vk::DeviceSize size);
} // namespace vulkan_core
