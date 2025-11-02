#include "gfx/vulkan_core.h"
#include "gravwll/third_party/sdl3/include/SDL3/SDL_video.h"
#include "gravwll/third_party/sdl3/include/SDL3/SDL_vulkan.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_wayland.h>
// #include <vulkan/vulkan_xlib.h>

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
      debug::debug_print("Validation layers acquired");
    }

    // 0. Flags
    uint32_t ext_q = 0;
    const char *const *res = SDL_Vulkan_GetInstanceExtensions(&ext_q);
    if (!res || ext_q == 0) {
      throw std::runtime_error(
          "SDL_Vulkan_GetInstanceExtensions failed or returned 0 extensions");
    }

    // скопировать в std::vector (удобнее добавлять свои)
    std::vector<const char *> extensions(res, res + ext_q);

    // получить текущий видеодрайвер
    const char *video_driver = SDL_GetCurrentVideoDriver();
    debug::debug_print("Current video driver: {}", video_driver);

    // добавить нужный surface-extension
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

    // выводим, что получилось
    for (auto e : extensions) {
      debug::debug_print("Using extension: {}", e);
    }

    // 1. Instance
    vk::ApplicationInfo appInfo{appName, 1, "Vulkan!", 1, VK_API_VERSION_1_3};
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

void init_frames(VulkanCore &core) {
  assert(core.swapchainImages.size() >= IN_FLIGHT_FRAME_COUNT);

  for (auto &frame : core.frames) {
    vk::CommandBufferAllocateInfo allocInfo{
        *core.commandPool, vk::CommandBufferLevel::ePrimary, 1};
    frame.commandBuffer =
        std::move(core.device.allocateCommandBuffers(allocInfo).front());

    // Syncs
    vk::SemaphoreCreateInfo semInfo{};
    frame.imageAvailableSemaphore = core.device.createSemaphore(semInfo);
    frame.renderFinishedSemaphore = core.device.createSemaphore(semInfo);

    vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
    frame.fence = core.device.createFence(fenceInfo);
  }
  debug::debug_print("{} frames inited", core.frames.size());
}

error::Result<bool> init_swapchain(VulkanCore &core, window::MyWindow &window) {
  // SwapChainSupportDetails swap_chain_support = query_swapchain_support(core);
  int width, height;
  SDL_GetWindowSizeInPixels(window.instance.get(), &width, &height);
  vk::Extent2D extent{static_cast<uint32_t>(width),
                      static_cast<uint32_t>(height)};
  core.swapchainExtent = extent;

  // 2️⃣ Получаем surface capabilities
  vk::SurfaceCapabilitiesKHR capabilities =
      core.physicalDevice.getSurfaceCapabilitiesKHR(*core.surface);

  auto formats = core.physicalDevice.getSurfaceFormatsKHR(*core.surface);
  auto presentModes =
      core.physicalDevice.getSurfacePresentModesKHR(*core.surface);

  if (formats.empty() || presentModes.empty()) {
    throw std::runtime_error(
        "Surface does not support formats or present modes");
  }

  // 3️⃣ Выбираем формат
  vk::SurfaceFormatKHR chosenFormat = formats[0];
  for (auto &f : formats) {
    if (f.format == vk::Format::eB8G8R8A8Srgb &&
        f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      chosenFormat = f;
      break;
    }
  }
  core.swapchainImageFormat = chosenFormat.format;
  vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
  for (auto &pm : presentModes) {
    if (pm == vk::PresentModeKHR::eMailbox) {
      presentMode = pm;
      break;
    }
  }

  uint32_t imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    imageCount = capabilities.maxImageCount;

  vk::SwapchainCreateInfoKHR swapInfo{};
  swapInfo.surface = *core.surface;
  swapInfo.minImageCount = imageCount;
  swapInfo.imageFormat = chosenFormat.format;
  swapInfo.imageColorSpace = chosenFormat.colorSpace;
  swapInfo.imageExtent = capabilities.currentExtent.width == UINT32_MAX
                             ? extent
                             : capabilities.currentExtent;
  swapInfo.imageArrayLayers = 1;
  swapInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

  uint32_t queueFamilyIndices[] = {core.graphicsQueueFamilyIndex};
  swapInfo.imageSharingMode = vk::SharingMode::eExclusive;
  swapInfo.queueFamilyIndexCount = 1;
  swapInfo.pQueueFamilyIndices = queueFamilyIndices;

  swapInfo.preTransform = capabilities.currentTransform;
  swapInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  swapInfo.presentMode = presentMode;
  swapInfo.clipped = VK_TRUE;
  swapInfo.oldSwapchain = nullptr;

  core.swapchain = vk::raii::SwapchainKHR(core.device, swapInfo);
  core.swapchainImages = core.swapchain.getImages();
  core.currentSwapchainImageIndex = 0;

  debug::debug_print("Swapchain created: {} images, format={}, extent={}x{}",
                     core.swapchainImages.size(),
                     static_cast<int>(core.swapchainImageFormat),
                     core.swapchainExtent.width, core.swapchainExtent.height);
  return error::Result<bool>::success(true);
}

bool check_validation_layer_support() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validation_layers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

SwapChainSupportDetails query_swapchain_support(VulkanCore &core) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*core.physicalDevice, *core.surface,
                                            &details.capabilities);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(*core.physicalDevice, *core.surface,
                                       &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(*core.physicalDevice, *core.surface,
                                         &format_count, details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(*core.physicalDevice, *core.surface,
                                            &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.presentModes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        *core.physicalDevice, *core.surface, &present_mode_count,
        details.presentModes.data());
  }
  return details;
}

} // namespace vulkan_core
