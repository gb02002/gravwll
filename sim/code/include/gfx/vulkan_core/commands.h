#pragma once

#include "device.h"
#include "utils/namespaces/error_namespace.h"

namespace vulkan_core {
void init_frames(VulkanCore &core);
error::Result<bool> record_frame_command_buffer(VulkanCore &core, Frame &frame,
                                                uint32_t imageIndex);

error::Result<bool> create_uniform_buffers(VulkanCore &core);
error::Result<bool> create_descriptor_pool_and_sets(VulkanCore &core);
error::Result<bool> create_vertex_buffer(VulkanCore &core);
} // namespace vulkan_core
