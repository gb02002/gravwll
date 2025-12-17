#pragma once

#include "device.h"
#include "utils/namespaces/error_namespace.h"

namespace vulkan_core {

error::Result<bool> create_graphics_pipeline(VulkanCore &);
error::Result<bool> create_render_pass(VulkanCore &);
} // namespace vulkan_core
