#include "gfx/vulkan_core/swapchain.h"
#include "utils/namespaces/error_namespace.h"

namespace vulkan_core {

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
    if (pm == vk::PresentModeKHR::eFifo) {
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
  debug::debug_print("Swapchain is inited");
  return error::Result<bool>::success(true);
}

error::Result<bool> create_image_views(VulkanCore &core) {
  try {
    core.swapchain_image_views.reserve(core.swapchainImages.size());

    for (size_t i = 0; i < core.swapchainImages.size(); i++) {
      vk::ImageViewCreateInfo createInfo{};
      createInfo.image = core.swapchainImages[i];
      createInfo.viewType = vk::ImageViewType::e2D;
      createInfo.format = core.swapchainImageFormat;

      // Каналы цвета (RGBA)
      createInfo.components.r = vk::ComponentSwizzle::eIdentity;
      createInfo.components.g = vk::ComponentSwizzle::eIdentity;
      createInfo.components.b = vk::ComponentSwizzle::eIdentity;
      createInfo.components.a = vk::ComponentSwizzle::eIdentity;

      // Что включаем в изображение (только цвет)
      createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      core.swapchain_image_views.emplace_back(core.device, createInfo);
    }

    debug::debug_print("Created {} image views",
                       core.swapchain_image_views.size());
    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create image views: {}", e.what());
    return error::Result<bool>::error(-1, "Failed to create image views");
  }
}

error::Result<bool> create_framebuffers(VulkanCore &core) {
  try {
    core.swapchain_frame_buffers.reserve(core.swapchain_image_views.size());

    for (size_t i = 0; i < core.swapchain_image_views.size(); i++) {
      // Массив attachments (в нашем случае - один color attachment)
      vk::ImageView attachments[] = {*core.swapchain_image_views[i]};

      vk::FramebufferCreateInfo framebufferInfo{};
      framebufferInfo.renderPass = *core.render_pass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = core.swapchainExtent.width;
      framebufferInfo.height = core.swapchainExtent.height;
      framebufferInfo.layers = 1;

      core.swapchain_frame_buffers.emplace_back(core.device, framebufferInfo);
    }

    debug::debug_print("Created {} framebuffers",
                       core.swapchain_frame_buffers.size());
    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create framebuffers: {}", e.what());
    return error::Result<bool>::error(-1, "Failed to create framebuffers");
  }
}

} // namespace vulkan_core
