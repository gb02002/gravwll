#include "utils/namespaces/error_namespace.h"
#include <cassert>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_wayland.h>

namespace vulkan_core {
const std::vector<const char *> validation_layers = {
    "VK_LAYER_KHRONOS_validation"};

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

} // namespace vulkan_core
