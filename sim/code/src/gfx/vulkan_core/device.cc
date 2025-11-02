#include "utils/namespaces/error_namespace.h"
#include <gfx/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

error::CResult<vk::raii::Device> get_device() {
  return error::CResult<vk::raii::Device>::error(1, "Not implemented");
};

error::CResult<vk::raii::PhysicalDevices> get_physical_device() {
  return error::CResult<vk::raii::PhysicalDevices>::error(1, "Not implemented");
};
