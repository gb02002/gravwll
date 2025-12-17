#pragma once

#include "device.h"
#include "utils/namespaces/error_namespace.h"

namespace vulkan_core {

error::Result<bool> init_swapchain(VulkanCore &core, window::MyWindow &window);
error::Result<bool> create_image_views(VulkanCore &core);
error::Result<bool> create_framebuffers(VulkanCore &core);
} // namespace vulkan_core
