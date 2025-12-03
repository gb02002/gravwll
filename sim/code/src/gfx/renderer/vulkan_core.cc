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
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_wayland.h>
// #include <vulkan/vulkan_xlib.h>

namespace vulkan_core {
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

VkShaderModule create_shader_model(VulkanCore &core,
                                   const std::vector<char> &code) {
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());
  VkShaderModule shader_module;
  if (vkCreateShaderModule(*core.device, &create_info, nullptr,
                           &shader_module) != VK_SUCCESS) {
    throw std::runtime_error("failed to create VkShaderModule");
  }
  return shader_module;
}

error::Result<bool> create_graphics_pipeline(VulkanCore &core) {
  auto vert_shader = load_shader("../assets/shaders/vertex.spv");
  VkShaderModule vert_shader_module = create_shader_model(core, vert_shader);

  VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
  vert_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vert_shader_module;
  vert_shader_stage_info.pName = "main";

  auto frag_shader = load_shader("../assets/shaders/fragment.spv");
  VkShaderModule frag_shader_module = create_shader_model(core, frag_shader);

  VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
  frag_shader_stage_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shader_module;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info,
                                                     frag_shader_stage_info};

  // Dynamic Parameters for Pipeline
  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                               VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkVertexInputBindingDescription binding_description{};
  binding_description.binding = 0;
  binding_description.stride = sizeof(Vertex);
  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription attr_pos{};
  attr_pos.binding = 0;
  attr_pos.location = 0;
  attr_pos.format = VK_FORMAT_R32G32B32_SFLOAT;
  attr_pos.offset = offsetof(Vertex, x);

  VkVertexInputAttributeDescription attr_mass{};
  attr_mass.binding = 0;
  attr_mass.location = 1;
  attr_mass.format = VK_FORMAT_R32_SFLOAT;
  attr_mass.offset = offsetof(Vertex, mass);

  std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = {
      attr_pos, attr_mass};

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.pVertexBindingDescriptions = &binding_description;
  vertex_input_info.vertexAttributeDescriptionCount =
      attribute_descriptions.size();
  vertex_input_info.pVertexAttributeDescriptions =
      attribute_descriptions.data();

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)core.swapchainExtent.width;
  viewport.height = (float)core.swapchainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = core.swapchainExtent;

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  // Colorblending. I think just for particles we don't need it
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  // Descriptor Set Layout
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSetLayoutBinding ubo_layout_binding;
  ubo_layout_binding.binding = 0;
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.descriptorCount = 1;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = 1;
  layout_info.pBindings = &ubo_layout_binding;

  vkCreateDescriptorSetLayout(*core.device, &layout_info, nullptr,
                              &descriptor_set_layout);
  VkPipelineLayout pipeline_layout;
  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
  if (vkCreatePipelineLayout(*core.device, &pipeline_layout_info, nullptr,
                             &pipeline_layout) != VK_SUCCESS)
    return error::Result<bool>::error(-1, "Failed to create pipeline_layout");

  // TODO: fininsh Pipeline initialization after finishing UBO for camera matrix
  // and RenderPass
  VkGraphicsPipelineCreateInfo pipeline_create_info{};
  pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_create_info.stageCount = 2;
  pipeline_create_info.pStages = shader_stages;

  pipeline_create_info.pVertexInputState = &vertex_input_info;
  pipeline_create_info.pInputAssemblyState = &input_assembly;
  pipeline_create_info.pViewportState = &viewport_state;
  pipeline_create_info.pRasterizationState = &rasterizer;
  pipeline_create_info.pMultisampleState = &multisampling;
  pipeline_create_info.pDepthStencilState = nullptr;
  pipeline_create_info.pColorBlendState = &colorBlending;
  pipeline_create_info.pDynamicState = &dynamicState;
  pipeline_create_info.layout = pipeline_layout;
  pipeline_create_info.renderPass = core.render_pass;
  pipeline_create_info.subpass = 0;

  if (vkCreateGraphicsPipelines(*core.device, VK_NULL_HANDLE, 1,
                                &pipeline_create_info, nullptr,
                                &core.gfx_pipeline) != VK_SUCCESS)
    return error::Result<bool>::error(-1, "Failed to init gfx_pipeline");

  vkDestroyShaderModule(*core.device, frag_shader_module, nullptr);
  vkDestroyShaderModule(*core.device, vert_shader_module, nullptr);
  return error::Result<bool>::success(1);
}

error::Result<bool> create_render_pass(VulkanCore &core) {
  VkAttachmentDescription color_attachment{};
  color_attachment.format = static_cast<VkFormat>(core.swapchainImageFormat);
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_create_info{};
  render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_create_info.attachmentCount = 1;
  render_pass_create_info.pAttachments = &color_attachment;
  render_pass_create_info.subpassCount = 1;
  render_pass_create_info.pSubpasses = &subpass;
  if (vkCreateRenderPass(*core.device, &render_pass_create_info, nullptr,
                         core.render_pass) != VK_SUCCESS)
    return error::Result<bool>::error(-1, "Failed to create render_pass");
  return error::Result<bool>::success(0);
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
} // namespace vulkan_core
