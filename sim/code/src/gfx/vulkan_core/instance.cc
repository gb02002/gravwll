// #define VULKAN_HPP_NO_EXCEPTIONS
// #define VULKAN_HPP_ASSERT_ON_RESULT false

#include "utils/namespaces/error_namespace.h"
#include <gfx/vulkan_core.h>
#include <utility>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

// error::CResult<vk::raii::Instance> vulkan_core::get_instance(const char
// *name) {
//   if (enableValidationLayers && !check_validation_layer_support()) {
//     return error::CResult<vk::raii::Instance>::error(
//         1, "Validation layers are not available");
//   } else {
//     debug::debug_print("Validation layers acquired");
//   }
//   try {
//     vk::raii::Context context;
//
//     vk::ApplicationInfo applicationInfo(name, 1, "Vulkan", 1,
//                                         VK_API_VERSION_1_1);
//
//     vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo);
//
//     vk::raii::Instance instance(context, instanceCreateInfo);
//     return error::CResult<vk::raii::Instance>::success(std::move(instance));
//
//   } catch (vk::SystemError &err) {
//     std::cout << "vk::SystemError: " << err.what() << std::endl;
//     return error::CResult<vk::raii::Instance>::error(
//         1, "Failed to create instance");
//   } catch (std::exception &err) {
//     std::cout << "std::exception: " << err.what() << std::endl;
//     return error::CResult<vk::raii::Instance>::error(
//         1, "Failed to create instance");
//   } catch (...) {
//     std::cout << "unknown error\n";
//     return error::CResult<vk::raii::Instance>::error(
//         1, "Failed to create instance");
//   }
// };
