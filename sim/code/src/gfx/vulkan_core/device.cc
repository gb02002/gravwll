#include "SDL3/SDL_vulkan.h"
#include "gfx/vulkan_core/utils.h"
#include "utils/namespaces/error_namespace.h"
#include <cstdint>
#include <gfx/vulkan_core/device.h>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_wayland.h>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;

#endif // NDEBUG

namespace vulkan_core {
error::CResult<VulkanCore> create_vulkan_core(const char *appName,
                                              window::MyWindow &window) {
  try {
    VulkanCore core{};
    core.context = vk::raii::Context{};
    if (enableValidationLayers && !check_validation_layer_support()) {
      return error::CResult<VulkanCore>::error(
          1, "Validation layers are not available");
    } else {
      std::cout << "Validation layers acquired" << std::endl;
    }

    uint32_t ext_q = 0;
    const char *const *res = SDL_Vulkan_GetInstanceExtensions(&ext_q);
    if (!res || ext_q == 0) {
      throw std::runtime_error(
          "SDL_Vulkan_GetInstanceExtensions failed or returned 0 extensions");
    }

    std::vector<const char *> extensions(res, res + ext_q);

    const char *video_driver = SDL_GetCurrentVideoDriver();
    debug::debug_print("Current video driver: {}", video_driver);

    if (strcmp(video_driver, "wayland") == 0) {
      extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
      //   vulkan_xlib naming is corrupted
      // } else if (strcmp(video_driver, "x11") == 0) {
      //   extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME); #
    } else if (strcmp(video_driver, "kmsdrm") == 0) {
      extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
    } else {
      // fallback — SDL может сделать offscreen surface
      debug::debug_print(
          "Warning: unknown video driver {}, using VK_KHR_surface only",
          video_driver);
    }

    for (auto e : extensions) {
      debug::debug_print("Using extension: {}", e);
    }

    // 1. Instance
    vk::ApplicationInfo appInfo{appName, 1, "Vulkan!", 1, VK_API_VERSION_1_4};
    vk::InstanceCreateInfo ici({}, &appInfo);
    ici.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ici.ppEnabledExtensionNames = extensions.data();
    // ici.ppEnabledExtensionNames = extensions.data();

    core.instance = vk::raii::Instance(core.context, ici);

    // 2. Surface
    core.surface = window.create_surface(core.instance).unwrap();

    // 3. Physical device
    auto physicalDevices = core.instance.enumeratePhysicalDevices();
    core.physicalDevice = physicalDevices.front();

    // 3.5 Query physical device features
    physical_device_features(core.physicalDevice);
    vk::PhysicalDeviceFeatures vulkan_features{};
    vulkan_features.shaderTessellationAndGeometryPointSize =
        core.physicalDevice.getFeatures()
            .shaderTessellationAndGeometryPointSize;
    vulkan_features.largePoints = core.physicalDevice.getFeatures().largePoints;

    // 4. Queue family
    auto queueFamilies = core.physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
      if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
        if (core.physicalDevice.getSurfaceSupportKHR(i, *core.surface))
          core.graphicsQueueFamilyIndex = i;
      }
    }

    // 5. Device
    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = core.graphicsQueueFamilyIndex;
    std::array queuePriorities{1.0f};
    queueCreateInfo.setQueuePriorities(queuePriorities);

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.pEnabledFeatures = &vulkan_features;

    std::array queueCreateInfos{queueCreateInfo};
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
    std::array<const char *const, 1> enabledExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    deviceCreateInfo.setPEnabledExtensionNames(enabledExtensions);

    vk::PhysicalDeviceVulkan13Features vulkan13Features{};
    vulkan13Features.synchronization2 = true;

    vk::StructureChain chain{deviceCreateInfo, vulkan13Features};

    core.device = vk::raii::Device(core.physicalDevice,
                                   chain.get<vk::DeviceCreateInfo>());

    // 6. Queue
    core.graphicsQueue =
        vk::raii::Queue(core.device, core.graphicsQueueFamilyIndex, 0);

    // 7. Command pool
    vk::CommandPoolCreateInfo cpci{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        core.graphicsQueueFamilyIndex};
    core.commandPool = vk::raii::CommandPool(core.device, cpci);

    return error::CResult<VulkanCore>::success(std::move(core));
  } catch (const std::exception &e) {
    debug::debug_print("Error in create_vulkan_core: {}", e.what());
    return error::CResult<VulkanCore>::error(1, e.what());
  }
}

inline void *map_memory(VulkanCore &core, vk::raii::DeviceMemory &memory,
                        vk::DeviceSize size, vk::DeviceSize offset = 0) {
  vk::MemoryMapInfo map_info{};
  map_info.memory = *memory;
  map_info.offset = offset;
  map_info.size = size;
  map_info.flags = vk::MemoryMapFlags{};

  uint8_t *map_result =
      static_cast<uint8_t *>(core.device.mapMemory2(map_info));
  if (!map_result) {
    throw std::runtime_error("Can't map memory");
  }
  return map_result;
}

inline void unmap_memory(VulkanCore &core, vk::raii::DeviceMemory &memory) {
  vk::MemoryUnmapInfo unmap_info{};
  unmap_info.memory = *memory;
  core.device.unmapMemory2(unmap_info);
}

void VulkanCore::clean_up() {
  if (*device) {
    device.waitIdle();
  }

  swapchain_frame_buffers.clear();
  swapchain_image_views.clear();

  for (auto &frame : frames) {
    frame.commandBuffer = nullptr;
    frame.imageAvailableSemaphore = nullptr;
    frame.renderFinishedSemaphore = nullptr;
    frame.fence = nullptr;
    frame.uniform_buffer = nullptr;
    frame.uniform_buffer_memory = nullptr;
    frame.descriptor_set = nullptr;
  }

  descriptor_pool = nullptr;

  gfx_pipeline = nullptr;
  pipeline_layout = nullptr;
  descriptor_set_layout = nullptr;
  render_pass = nullptr;
  vertex_buffer = nullptr;
  vertex_buffer_memory = nullptr;
  swapchain = nullptr;
  commandPool = nullptr;
  device = nullptr;
  surface = nullptr;
  instance = nullptr;
}

} // namespace vulkan_core
