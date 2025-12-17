#pragma once

#include "device.h"
#include "utils/namespaces/error_namespace.h"

namespace vulkan_core {
bool check_validation_layer_support();
error::Result<bool>
physical_device_features(const vk::raii::PhysicalDevice p_device);
SwapChainSupportDetails query_swapchain_support(VulkanCore &core);
} // namespace vulkan_core
