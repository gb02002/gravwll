#include "gfx/vulkan_core.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "ctx/config.h"
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
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_wayland.h>

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
  glm::vec3 pos;
  float mass;
};

error::Result<bool>
physical_device_features(const vk::raii::PhysicalDevice p_device) {
  std::vector<vk::ExtensionProperties> extension_properties =
      p_device.enumerateDeviceExtensionProperties();
  std::cout << "PhysicalDevice features" << "\n";

  {
    auto features2 = p_device.getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDevice16BitStorageFeatures,
        vk::PhysicalDevice8BitStorageFeaturesKHR,
        vk::PhysicalDeviceASTCDecodeFeaturesEXT,
        vk::PhysicalDeviceBlendOperationAdvancedFeaturesEXT,
        vk::PhysicalDeviceBufferDeviceAddressFeaturesEXT,
        vk::PhysicalDeviceCoherentMemoryFeaturesAMD,
        vk::PhysicalDeviceComputeShaderDerivativesFeaturesNV,
        vk::PhysicalDeviceConditionalRenderingFeaturesEXT,
        vk::PhysicalDeviceCooperativeMatrixFeaturesNV,
        vk::PhysicalDeviceCornerSampledImageFeaturesNV,
        vk::PhysicalDeviceCoverageReductionModeFeaturesNV,
        vk::PhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV,
        vk::PhysicalDeviceDepthClipEnableFeaturesEXT,
        vk::PhysicalDeviceDescriptorIndexingFeaturesEXT,
        vk::PhysicalDeviceExclusiveScissorFeaturesNV,
        vk::PhysicalDeviceFragmentDensityMapFeaturesEXT,
        vk::PhysicalDeviceFragmentShaderBarycentricFeaturesNV,
        vk::PhysicalDeviceFragmentShaderInterlockFeaturesEXT,
        vk::PhysicalDeviceHostQueryResetFeaturesEXT,
        vk::PhysicalDeviceImagelessFramebufferFeaturesKHR,
        vk::PhysicalDeviceIndexTypeUint8FeaturesEXT,
        vk::PhysicalDeviceInlineUniformBlockFeaturesEXT,
        vk::PhysicalDeviceLineRasterizationFeaturesEXT,
        vk::PhysicalDeviceMemoryPriorityFeaturesEXT,
        vk::PhysicalDeviceMeshShaderFeaturesNV,
        vk::PhysicalDeviceMultiviewFeatures,
        vk::PhysicalDevicePipelineExecutablePropertiesFeaturesKHR,
        vk::PhysicalDeviceProtectedMemoryFeatures,
        vk::PhysicalDeviceRepresentativeFragmentTestFeaturesNV,
        vk::PhysicalDeviceSamplerYcbcrConversionFeatures,
        vk::PhysicalDeviceScalarBlockLayoutFeaturesEXT,
        vk::PhysicalDeviceShaderAtomicInt64FeaturesKHR,
        vk::PhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT,
        vk::PhysicalDeviceShaderDrawParametersFeatures,
        vk::PhysicalDeviceShaderFloat16Int8FeaturesKHR,
        vk::PhysicalDeviceShaderImageFootprintFeaturesNV,
        vk::PhysicalDeviceShaderIntegerFunctions2FeaturesINTEL,
        vk::PhysicalDeviceShaderSMBuiltinsFeaturesNV,
        vk::PhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR,
        vk::PhysicalDeviceShadingRateImageFeaturesNV,
        vk::PhysicalDeviceSubgroupSizeControlFeaturesEXT,
        vk::PhysicalDeviceTexelBufferAlignmentFeaturesEXT,
        vk::PhysicalDeviceTextureCompressionASTCHDRFeaturesEXT,
        vk::PhysicalDeviceTimelineSemaphoreFeaturesKHR,
        vk::PhysicalDeviceTransformFeedbackFeaturesEXT,
        vk::PhysicalDeviceUniformBufferStandardLayoutFeaturesKHR,
        vk::PhysicalDeviceVariablePointersFeatures,
        vk::PhysicalDeviceVertexAttributeDivisorFeaturesEXT,
        vk::PhysicalDeviceVulkanMemoryModelFeaturesKHR,
        vk::PhysicalDeviceYcbcrImageArraysFeaturesEXT>();
    vk::PhysicalDeviceFeatures const &features =
        features2.get<vk::PhysicalDeviceFeatures2>().features;
    std::cout << "\tFeatures:\n";
    std::cout << "\t\talphaToOne                              : "
              << !!features.alphaToOne << "\n";
    std::cout << "\t\tdepthBiasClamp                          : "
              << !!features.depthBiasClamp << "\n";
    std::cout << "\t\tdepthBounds                             : "
              << !!features.depthBounds << "\n";
    std::cout << "\t\tdepthClamp                              : "
              << !!features.depthClamp << "\n";
    std::cout << "\t\tdrawIndirectFirstInstance               : "
              << !!features.drawIndirectFirstInstance << "\n";
    std::cout << "\t\tdualSrcBlend                            : "
              << !!features.dualSrcBlend << "\n";
    std::cout << "\t\tfillModeNonSolid                        : "
              << !!features.fillModeNonSolid << "\n";
    std::cout << "\t\tfragmentStoresAndAtomics                : "
              << !!features.fragmentStoresAndAtomics << "\n";
    std::cout << "\t\tfullDrawIndexUint32                     : "
              << !!features.fullDrawIndexUint32 << "\n";
    std::cout << "\t\tgeometryShader                          : "
              << !!features.geometryShader << "\n";
    std::cout << "\t\timageCubeArray                          : "
              << !!features.imageCubeArray << "\n";
    std::cout << "\t\tindependentBlend                        : "
              << !!features.independentBlend << "\n";
    std::cout << "\t\tinheritedQueries                        : "
              << !!features.inheritedQueries << "\n";
    std::cout << "\t\tlargePoints                             : "
              << !!features.largePoints << "\n";
    std::cout << "\t\tlogicOp                                 : "
              << !!features.logicOp << "\n";
    std::cout << "\t\tmultiDrawIndirect                       : "
              << !!features.multiDrawIndirect << "\n";
    std::cout << "\t\tmultiViewport                           : "
              << !!features.multiViewport << "\n";
    std::cout << "\t\tocclusionQueryPrecise                   : "
              << !!features.occlusionQueryPrecise << "\n";
    std::cout << "\t\tpipelineStatisticsQuery                 : "
              << !!features.pipelineStatisticsQuery << "\n";
    std::cout << "\t\trobustBufferAccess                      : "
              << !!features.robustBufferAccess << "\n";
    std::cout << "\t\tsamplerAnisotropy                       : "
              << !!features.samplerAnisotropy << "\n";
    std::cout << "\t\tsampleRateShading                       : "
              << !!features.sampleRateShading << "\n";
    std::cout << "\t\tshaderClipDistance                      : "
              << !!features.shaderClipDistance << "\n";
    std::cout << "\t\tshaderCullDistance                      : "
              << !!features.shaderCullDistance << "\n";
    std::cout << "\t\tshaderFloat64                           : "
              << !!features.shaderFloat64 << "\n";
    std::cout << "\t\tshaderImageGatherExtended               : "
              << !!features.shaderImageGatherExtended << "\n";
    std::cout << "\t\tshaderInt16                             : "
              << !!features.shaderInt16 << "\n";
    std::cout << "\t\tshaderInt64                             : "
              << !!features.shaderInt64 << "\n";
    std::cout << "\t\tshaderResourceMinLod                    : "
              << !!features.shaderResourceMinLod << "\n";
    std::cout << "\t\tshaderResourceResidency                 : "
              << !!features.shaderResourceResidency << "\n";
    std::cout << "\t\tshaderSampledImageArrayDynamicIndexing  : "
              << !!features.shaderSampledImageArrayDynamicIndexing << "\n";
    std::cout << "\t\tshaderStorageBufferArrayDynamicIndexing : "
              << !!features.shaderStorageBufferArrayDynamicIndexing << "\n";
    std::cout << "\t\tshaderStorageImageArrayDynamicIndexing  : "
              << !!features.shaderStorageImageArrayDynamicIndexing << "\n";
    std::cout << "\t\tshaderStorageImageExtendedFormats       : "
              << !!features.shaderStorageImageExtendedFormats << "\n";
    std::cout << "\t\tshaderStorageImageMultisample           : "
              << !!features.shaderStorageImageMultisample << "\n";
    std::cout << "\t\tshaderStorageImageReadWithoutFormat     : "
              << !!features.shaderStorageImageReadWithoutFormat << "\n";
    std::cout << "\t\tshaderStorageImageWriteWithoutFormat    : "
              << !!features.shaderStorageImageWriteWithoutFormat << "\n";
    std::cout << "\t\tshaderTessellationAndGeometryPointSize  : "
              << !!features.shaderTessellationAndGeometryPointSize << "\n";
    std::cout << "\t\tshaderUniformBufferArrayDynamicIndexing : "
              << !!features.shaderUniformBufferArrayDynamicIndexing << "\n";
    std::cout << "\t\tsparseBinding                           : "
              << !!features.sparseBinding << "\n";
    std::cout << "\t\tsparseResidency16Samples                : "
              << !!features.sparseResidency16Samples << "\n";
    std::cout << "\t\tsparseResidency2Samples                 : "
              << !!features.sparseResidency2Samples << "\n";
    std::cout << "\t\tsparseResidency4Samples                 : "
              << !!features.sparseResidency4Samples << "\n";
    std::cout << "\t\tsparseResidency8Samples                 : "
              << !!features.sparseResidency8Samples << "\n";
    std::cout << "\t\tsparseResidencyAliased                  : "
              << !!features.sparseResidencyAliased << "\n";
    std::cout << "\t\tsparseResidencyBuffer                   : "
              << !!features.sparseResidencyBuffer << "\n";
    std::cout << "\t\tsparseResidencyImage2D                  : "
              << !!features.sparseResidencyImage2D << "\n";
    std::cout << "\t\tsparseResidencyImage3D                  : "
              << !!features.sparseResidencyImage3D << "\n";
    std::cout << "\t\ttessellationShader                      : "
              << !!features.tessellationShader << "\n";
    std::cout << "\t\ttextureCompressionASTC_LDR              : "
              << !!features.textureCompressionASTC_LDR << "\n";
    std::cout << "\t\ttextureCompressionBC                    : "
              << !!features.textureCompressionBC << "\n";
    std::cout << "\t\ttextureCompressionETC2                  : "
              << !!features.textureCompressionETC2 << "\n";
    std::cout << "\t\tvariableMultisampleRate                 : "
              << !!features.variableMultisampleRate << "\n";
    std::cout << "\t\tvertexPipelineStoresAndAtomics          : "
              << !!features.vertexPipelineStoresAndAtomics << "\n";
    std::cout << "\t\twideLines                               : "
              << !!features.wideLines << "\n";
    std::cout << "\n";
    return error::Result<bool>::success(true);
  }
  auto properties = p_device.getProperties();
  debug::debug_print("Point size range: {} - {}",
                     properties.limits.pointSizeRange[0],
                     properties.limits.pointSizeRange[1]);
  debug::debug_print("Point size granularity: {}",
                     properties.limits.pointSizeGranularity);
}

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

uint32_t find_memory_type(VulkanCore &core, uint32_t type_filter,
                          vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties mem_properties =
      core.physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
    if ((type_filter & (1 << i)) &&
        (mem_properties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type");
}

void create_buffer(VulkanCore &core, vk::DeviceSize size,
                   vk::BufferUsageFlags usage,
                   vk::MemoryPropertyFlags properties, vk::raii::Buffer &buffer,
                   vk::raii::DeviceMemory &buffer_memory) {
  vk::BufferCreateInfo buffer_info{};
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = vk::SharingMode::eExclusive;

  buffer = vk::raii::Buffer(core.device, buffer_info);

  vk::MemoryRequirements mem_requirements = buffer.getMemoryRequirements();

  vk::MemoryAllocateInfo alloc_info;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex =
      find_memory_type(core, mem_requirements.memoryTypeBits, properties);

  buffer_memory = vk::raii::DeviceMemory(core.device, alloc_info);
  buffer.bindMemory(*buffer_memory, 0);
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
  debug::debug_print("Swapchain is inited");
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
    auto vertShader = load_shader("vertex.spv");
    auto fragShader = load_shader("fragment.spv");
    // auto vertShader = load_shader("../assets/shaders/vertex.spv");
    // auto fragShader = load_shader("../assets/shaders/fragment.spv");

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
    attr_pos.offset = offsetof(Vertex, pos);

    vk::VertexInputAttributeDescription attr_mass{};
    attr_mass.binding = 0;
    attr_mass.location = 1;
    attr_mass.format = vk::Format::eR32Sfloat;
    attr_mass.offset = offsetof(Vertex, mass);

    std::array<vk::VertexInputAttributeDescription, 2> attribute_descriptions =
        {attr_pos, attr_mass};

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions =
        attribute_descriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.topology = vk::PrimitiveTopology::ePointList;
    // input_assembly.topology = vk::PrimitiveTopology::eTriangleList;
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
    rasterizer.setRasterizerDiscardEnable(VK_FALSE);
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    // rasterizer.polygonMode = vk::PolygonMode::ePoint;
    rasterizer.lineWidth = 1.0f;
    // rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.cullMode = vk::CullModeFlagBits::eNone;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.alphaToCoverageEnable = VK_FALSE;

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
  std::string dir = std::string(SHADER_DIRECTORY);
  std::string filepath = dir.empty() ? filename : dir + "/" + filename;

  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

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

error::Result<bool> record_frame_command_buffer(VulkanCore &core, Frame &frame,
                                                uint32_t imageIndex) {
  try {
    std::cout << "Recording command buffer for image: " << imageIndex << "\n";
    std::cout << "swapchainExtent: " << core.swapchainExtent.width << "x"
              << core.swapchainExtent.height << "\n";

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    frame.commandBuffer.begin(beginInfo);

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

    std::array<vk::Viewport, 1> viewports = {viewport};
    std::array<vk::Rect2D, 1> scissors = {scissor};

    frame.commandBuffer.setViewport(0, viewports);
    frame.commandBuffer.setScissor(0, scissors);

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = *core.render_pass;
    renderPassInfo.framebuffer = *core.swapchain_frame_buffers[imageIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = core.swapchainExtent;

    vk::ClearValue clearColor =
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    frame.commandBuffer.beginRenderPass(renderPassInfo,
                                        vk::SubpassContents::eInline);

    // Привязываем пайплайн
    frame.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                     *core.gfx_pipeline);

    // Привязываем вершинный буфер - используем std::array для правильного
    // ArrayProxy
    std::array<vk::Buffer, 1> vertexBuffers = {*core.vertex_buffer};
    std::array<vk::DeviceSize, 1> offsets = {0};
    frame.commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    debug::debug_print("Drawing {} vertices", core.particle_count);

    // Привязываем descriptor set ИМЕННО ЭТОГО КАДРА (текущего frame)
    if (*frame.descriptor_set != nullptr) {
      std::array<vk::DescriptorSet, 1> descriptorSets = {*frame.descriptor_set};
      frame.commandBuffer.bindDescriptorSets(
          vk::PipelineBindPoint::eGraphics, *core.pipeline_layout, 0,
          descriptorSets, {} // пустой массив dynamicOffsets
      );
    } else {
      debug::debug_print("Warning: descriptor set is null for frame");
    }

    // Рисуем
    frame.commandBuffer.draw(static_cast<uint32_t>(core.particle_count), 1, 0,
                             0);

    frame.commandBuffer.endRenderPass();
    frame.commandBuffer.end();

    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    std::cout << "Failed to record frame command buffer" << e.what();
    return error::Result<bool>::error(-1,
                                      "Failed to record frame command buffer");
  }
}

error::Result<bool> create_uniform_buffers(VulkanCore &core) {
  try {
    // vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
    vk::DeviceSize bufferSize = sizeof(CameraUBO);

    for (auto &frame : core.frames) {
      create_buffer(core, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible |
                        vk::MemoryPropertyFlagBits::eHostCoherent,
                    frame.uniform_buffer, frame.uniform_buffer_memory);

      vk::MemoryMapInfo mapInfo{};
      mapInfo.memory = *frame.uniform_buffer_memory;
      mapInfo.offset = 0;
      mapInfo.size = bufferSize;
      mapInfo.flags = vk::MemoryMapFlags();

      frame.uniform_buffer_mapped = core.device.mapMemory2(mapInfo);

      debug::debug_print("Created uniform buffer (size: {})", bufferSize);
    }

    return error::Result<bool>::success(true);
  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create uniform buffers: {}", e.what());
    return error::Result<bool>::error(-1, "Failed to create uniform buffers");
  }
}

error::Result<bool> create_descriptor_pool_and_sets(VulkanCore &core) {
  try {
    // 1. Создать descriptor pool
    vk::DescriptorPoolSize poolSize{vk::DescriptorType::eUniformBuffer,
                                    IN_FLIGHT_FRAME_COUNT};
    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &poolSize;
    pool_info.maxSets = IN_FLIGHT_FRAME_COUNT;
    core.descriptor_pool = std::make_unique<vk::raii::DescriptorPool>(
        core.device.createDescriptorPool(pool_info));

    debug::debug_print("Descriptor pool created");

    // 2. Выделить descriptor sets для каждого кадра
    std::vector<vk::DescriptorSetLayout> layouts(IN_FLIGHT_FRAME_COUNT,
                                                 *core.descriptor_set_layout);
    vk::DescriptorSetAllocateInfo alloc_info{};
    alloc_info.descriptorPool = *core.descriptor_pool;
    alloc_info.descriptorSetCount = IN_FLIGHT_FRAME_COUNT;
    alloc_info.pSetLayouts = layouts.data();

    auto sets = core.device.allocateDescriptorSets(alloc_info);

    debug::debug_print("Allocated {} descriptor sets", sets.size());

    // 3. Привязать uniform буферы к descriptor sets
    for (size_t i = 0; i < core.frames.size(); i++) {
      vk::DescriptorBufferInfo buffer_info{};
      buffer_info.buffer = *core.frames[i].uniform_buffer;
      buffer_info.offset = 0;
      // buffer_info.range = sizeof(UniformBufferObject);
      buffer_info.range = sizeof(CameraUBO);

      vk::WriteDescriptorSet descriptor_write{};
      descriptor_write.dstSet = *sets[i];
      descriptor_write.dstBinding = 0;
      descriptor_write.dstArrayElement = 0;
      descriptor_write.descriptorType = vk::DescriptorType::eUniformBuffer;
      descriptor_write.descriptorCount = 1;
      descriptor_write.pBufferInfo = &buffer_info;

      core.device.updateDescriptorSets(descriptor_write, nullptr);

      // Сохранить descriptor set в Frame
      core.frames[i].descriptor_set = std::move(sets[i]);

      debug::debug_print("Bound uniform buffer to descriptor set {}", i);
    }

    return error::Result<bool>::success(true);
  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create descriptor pool and sets: {}",
                       e.what());
    return error::Result<bool>::error(
        -1, "Failed to create descriptor pool and sets");
  }
}

error::Result<bool> create_vertex_buffer(VulkanCore &core) {
  std::cout << "Vertex struct size: " << sizeof(Vertex) << ", "
            << "offset of pos: " << offsetof(Vertex, pos)
            << "offset of size: " << offsetof(Vertex, mass) << "\n";
  try {
    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, 1.0f}, // должен быть виден
        {{1.0f, -1.0f, 0.0f}, 2.0f},
        {{-1.0f, 1.0f, 0.0f}, 3.0f},
        {{1.0f, 1.0f, 0.0f}, 4.0f},
        {{0.0f, 0.0f, 0.0f}, 5.0f}};
    core.particle_count = vertices.size();
    vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    // Создаем временный staging буфер
    vk::raii::Buffer stagingBuffer{nullptr};
    vk::raii::DeviceMemory stagingBufferMemory{nullptr};

    create_buffer(core, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                  vk::MemoryPropertyFlagBits::eHostVisible |
                      vk::MemoryPropertyFlagBits::eHostCoherent,
                  stagingBuffer, stagingBufferMemory);

    vk::MemoryMapInfo mapInfo{};
    mapInfo.memory = *stagingBufferMemory;
    mapInfo.offset = 0;
    mapInfo.size = bufferSize;
    mapInfo.flags = vk::MemoryMapFlags();

    void *data = core.device.mapMemory2(mapInfo);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));

    // WARNING: same here
    vk::MemoryUnmapInfo unmapInfo{};
    unmapInfo.memory = *stagingBufferMemory;
    core.device.unmapMemory2(unmapInfo);

    // Создаем конечный vertex буфер
    create_buffer(core, bufferSize,
                  vk::BufferUsageFlagBits::eVertexBuffer |
                      vk::BufferUsageFlagBits::eTransferDst,
                  vk::MemoryPropertyFlagBits::eDeviceLocal, core.vertex_buffer,
                  core.vertex_buffer_memory);

    // Копируем из временного в конечный буфер
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = *core.commandPool;
    allocInfo.commandBufferCount = 1;

    auto commandBuffers = core.device.allocateCommandBuffers(allocInfo);
    vk::raii::CommandBuffer commandBuffer = std::move(commandBuffers[0]);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    commandBuffer.copyBuffer(*stagingBuffer, *core.vertex_buffer, copyRegion);

    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffer;

    core.graphicsQueue.submit(submitInfo, nullptr);
    core.graphicsQueue.waitIdle();

    debug::debug_print("Created vertex buffer with {} vertices",
                       core.particle_count);
    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create vertex buffer: {}", e.what());
    return error::Result<bool>::error(-1, "Failed to create vertex buffer");
  }
}
void copy_to_buffer(VulkanCore &core, vk::raii::DeviceMemory &dstMemory,
                    const void *data, vk::DeviceSize size) {
  // WARNING: LLM proposes MemoryMapInfo2 and MemoryUnmapInfo2
  vk::MemoryMapInfo mapInfo{};
  mapInfo.memory = *dstMemory;
  mapInfo.offset = 0;
  mapInfo.size = size;
  mapInfo.flags = vk::MemoryMapFlags();

  void *mappedData = core.device.mapMemory2(mapInfo);
  memcpy(mappedData, data, static_cast<size_t>(size));

  vk::MemoryUnmapInfo unmapInfo{};
  unmapInfo.memory = *dstMemory;
  core.device.unmapMemory2(unmapInfo);
}

void VulkanCore::clean_up() {
  // 1. Ждем завершения всех операций
  if (*device) {
    device.waitIdle();
  }

  // 2. Вручную зануляем объекты в правильном порядке
  swapchain_frame_buffers.clear();
  swapchain_image_views.clear();

  // 3. Уничтожаем frames вручную
  for (auto &frame : frames) {
    frame.commandBuffer = nullptr;
    frame.imageAvailableSemaphore = nullptr;
    frame.renderFinishedSemaphore = nullptr;
    frame.fence = nullptr;
    frame.uniform_buffer = nullptr;
    frame.uniform_buffer_memory = nullptr;
    frame.descriptor_set = nullptr;
  }

  // 4. Теперь можно уничтожать pool
  descriptor_pool = nullptr;

  // 5. Остальные объекты
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
