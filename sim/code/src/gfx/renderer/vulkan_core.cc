#include "gfx/vulkan_core.h"
#include "gravwll/third_party/sdl3/include/SDL3/SDL_video.h"
#include "gravwll/third_party/sdl3/include/SDL3/SDL_vulkan.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_wayland.h>
// #include <vulkan/vulkan_xlib.h>

const std::vector<const char *> validation_layers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;

#endif // NDEBUG

namespace vulkan_core {
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

struct Vertex {
  float x;
  float y;
  float z;
  float mass;
};

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
  return error::Result<bool>::success(true);
}

error::Result<bool> create_graphics_pipeline(VulkanCore &core) {
  debug::debug_print("Starting pipeline creation...");
  if (!*core.render_pass) {
    auto result = create_render_pass(core);
    if (result.is_error())
      return result;
  }

  debug::debug_print("Render pass created");
  try {
    auto vertShader = load_shader("../assets/shaders/vertex.spv");
    auto fragShader = load_shader("../assets/shaders/fragment.spv");

    vk::ShaderModuleCreateInfo vertShaderInfo{};
    vertShaderInfo.codeSize = vertShader.size();
    vertShaderInfo.pCode =
        reinterpret_cast<const uint32_t *>(vertShader.data());

    vk::ShaderModuleCreateInfo fragShaderInfo{};
    fragShaderInfo.codeSize = fragShader.size();
    fragShaderInfo.pCode =
        reinterpret_cast<const uint32_t *>(fragShader.data());

    vk::raii::ShaderModule vertShaderModule(core.device, vertShaderInfo);
    vk::raii::ShaderModule fragShaderModule(core.device, fragShaderInfo);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.setPNext(nullptr);
    vertShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = *vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.setPNext(nullptr);
    fragShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = *fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    vk::PipelineShaderStageCreateInfo shader_stages[] = {vertShaderStageInfo,
                                                         fragShaderStageInfo};
    std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport,
                                                    vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.dynamicStateCount =
        static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    vk::VertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = vk::VertexInputRate::eVertex;

    vk::VertexInputAttributeDescription attr_pos{};
    attr_pos.binding = 0;
    attr_pos.location = 0;
    attr_pos.format = vk::Format::eR32G32B32Sfloat;
    attr_pos.offset = offsetof(Vertex, x);

    vk::VertexInputAttributeDescription attr_mass{};
    attr_mass.binding = 0;
    attr_mass.location = 1;
    attr_mass.format = vk::Format::eR32Sfloat;
    attr_mass.offset = offsetof(Vertex, mass);

    std::array<vk::VertexInputAttributeDescription, 2> attribute_descriptions =
        {attr_mass, attr_mass};

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions =
        attribute_descriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.topology = vk::PrimitiveTopology::ePointList;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(core.swapchainExtent.width);
    viewport.height = static_cast<float>(core.swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = core.swapchainExtent;

    vk::PipelineViewportStateCreateInfo viewport_state{};
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::ePoint;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    color_blend_attachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo color_blending{};
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    vk::DescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutCreateInfo layout_info{};
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_layout_binding;

    core.descriptor_set_layout =
        vk::raii::DescriptorSetLayout(core.device, layout_info);

    vk::PipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &*core.descriptor_set_layout;

    core.pipeline_layout =
        vk::raii::PipelineLayout(core.device, pipeline_layout_info);

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shader_stages;
    pipelineInfo.pVertexInputState = &vertex_input_info;
    pipelineInfo.pInputAssemblyState = &input_assembly;
    pipelineInfo.pViewportState = &viewport_state;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &color_blending;
    pipelineInfo.pDynamicState = &dynamic_state;
    pipelineInfo.layout = *core.pipeline_layout;
    pipelineInfo.renderPass = *core.render_pass;
    pipelineInfo.subpass = 0;

    core.gfx_pipeline = vk::raii::Pipeline(core.device, nullptr, pipelineInfo);

    debug::debug_print("Graphics pipeline created successfully");
    return error::Result<bool>::success(true);
  } catch (vk::SystemError &error) {
    debug::debug_print("Error in create_graphics_pipeline: %s", error.what());
    return error::Result<bool>::error(-1, error.what());
  }
}

error::Result<bool> create_render_pass(VulkanCore &core) {
  try {
    vk::AttachmentDescription color_attachment{};
    color_attachment.format = core.swapchainImageFormat;
    color_attachment.samples = vk::SampleCountFlagBits::e1;
    color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    color_attachment.initialLayout = vk::ImageLayout::eUndefined;
    color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    vk::RenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;

    core.render_pass =
        vk::raii::RenderPass(core.device, render_pass_create_info);
    debug::debug_print("RenderPass created successfully");

    return error::Result<bool>::success(true);
  } catch (vk::SystemError &error) {
    debug::debug_print("Failed to create render pass: {}", error.what());
    return error::Result<bool>::error(-1, error.what());
  }
}

static std::vector<char> load_shader(const std::string &filename) {
  // std::cout << "Current path is " << std::filesystem::current_path() << '\n';
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    std::string err_msg = filename + ": cound not load";
    throw std::runtime_error(err_msg);
  }

  size_t file_size = (size_t)file.tellg();
  debug::debug_print("{} size is {}", filename, file_size);
  std::vector<char> buffer(file_size);

  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();

  return buffer;
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

error::Result<bool> create_command_buffers(VulkanCore &core) {
  try {
    core.command_buffers.reserve(core.swapchain_frame_buffers.size());

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *core.commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount =
        static_cast<uint32_t>(core.swapchain_frame_buffers.size());

    core.command_buffers = core.device.allocateCommandBuffers(allocInfo);

    debug::debug_print("Created {} command buffers for rendering",
                       core.command_buffers.size());
    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create command buffers: {}", e.what());
    return error::Result<bool>::error(-1, "Failed to create command buffers");
  }
}

void record_command_buffer(VulkanCore &core, vk::CommandBuffer commandBuffer,
                           uint32_t imageIndex) {
  // Начинаем запись
  vk::CommandBufferBeginInfo beginInfo{};
  beginInfo.flags =
      vk::CommandBufferUsageFlags(); // Можно использовать eOneTimeSubmit

  commandBuffer.begin(beginInfo);

  // Начинаем render pass
  vk::RenderPassBeginInfo renderPassInfo{};
  renderPassInfo.renderPass = *core.render_pass;
  renderPassInfo.framebuffer = *core.swapchain_frame_buffers[imageIndex];
  renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
  renderPassInfo.renderArea.extent = core.swapchainExtent;

  // Цвет очистки (черный)
  vk::ClearValue clearColor =
      vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  // Привязываем пайплайн
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             *core.gfx_pipeline);

  // TODO: Здесь будет привязка вершинного буфера и uniform буферов
  // Пока просто рисуем без данных
  commandBuffer.draw(3, 1, 0, 0); // Нарисовать 3 вершины (тест)

  // Заканчиваем render pass
  commandBuffer.endRenderPass();

  // Заканчиваем запись
  commandBuffer.end();
}

error::Result<bool> record_command_buffers(VulkanCore &core) {
  try {
    for (size_t i = 0; i < core.command_buffers.size(); i++) {
      record_command_buffer(core, *core.command_buffers[i],
                            static_cast<uint32_t>(i));
    }
    debug::debug_print("Recorded commands for {} command buffers",
                       core.command_buffers.size());
    return error::Result<bool>::success(true);
  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to record command buffers: {}", e.what());
    return error::Result<bool>::error(-1, "Failed to record command buffers");
  }
}

void create_uniform_buffers(VulkanCore &core) {
  for (auto &frame : core.frames) {
    // TODO: Создать uniform буфер
    // Выделить память
    // Отобразить в адресное пространство CPU
  }
}

void create_descriptor_pool_and_sets(VulkanCore &core) {
  // 1. Создать descriptor pool
  vk::DescriptorPoolSize poolSize{vk::DescriptorType::eUniformBuffer,
                                  IN_FLIGHT_FRAME_COUNT};
  vk::DescriptorPoolCreateInfo poolInfo{};
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = IN_FLIGHT_FRAME_COUNT;
  core.descriptor_pool = core.device.createDescriptorPool(poolInfo);

  // 2. Выделить descriptor sets для каждого кадра
  std::vector<vk::DescriptorSetLayout> layouts(IN_FLIGHT_FRAME_COUNT,
                                               *core.descriptor_set_layout);
  vk::DescriptorSetAllocateInfo allocInfo{};
  allocInfo.descriptorPool = *core.descriptor_pool;
  allocInfo.descriptorSetCount = IN_FLIGHT_FRAME_COUNT;
  allocInfo.pSetLayouts = layouts.data();

  auto sets = core.device.allocateDescriptorSets(allocInfo);

  // 3. Привязать uniform буферы к descriptor sets
  for (size_t i = 0; i < core.frames.size(); i++) {
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = *core.frames[i].uniform_buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.dstSet = *sets[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    core.device.updateDescriptorSets(descriptorWrite, nullptr);

    // Сохранить descriptor set в Frame
    core.frames[i].descriptorSet = std::move(sets[i]);
  }
}
} // namespace vulkan_core
