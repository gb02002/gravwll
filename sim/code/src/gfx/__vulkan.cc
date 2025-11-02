// // This file has a bunch of test code for new dependencies
//
// #define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 0
// #include "../../../third_party/VulkanMemoryAllocator/include/vk_mem_alloc.h"
// #include "../../../third_party/imgui/backends/imgui_impl_sdl3.h"
// #include "../../../third_party/vk-bootstrap/src/VkBootstrap.h"
// #include "SDL3/SDL_vulkan.h"
// #include "gfx/gfx.h"
// #include "gravwll/third_party/sdl3/include/SDL3/SDL_video.h"
// #include "imgui_impl_sdl3.h"
// #include "utils/namespaces/error_namespace.h"
// #include "vulkan/vulkan.hpp"
// #include <SDL3/SDL.h>
// #include <SDL3/SDL_events.h>
// #include <SDL3/SDL_init.h>
// #include <SDL3/SDL_oldnames.h>
// #include <SDL3/SDL_video.h>
// #include <cmath>
// #include <memory>
// #include <stdbool.h>
// #include <stdexcept>
// #include <string>
// #include <vulkan/vulkan_core.h>
// #include <vulkan/vulkan_raii.hpp>
//
// static std::string AppName = "01_InitInstance";
// static std::string EngineName = "qwe";
//
// void GfxEngine::init_vulkan1() {
//   debug::debug_print("Инициализация SDL3...");
//   // SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland");
//
//   int num_drivers = SDL_GetNumVideoDrivers();
//   debug::debug_print("Доступно видео драйверов: {}", num_drivers);
//
//   for (int i = 0; i < num_drivers; i++) {
//     debug::debug_print("  Драйвер {}: {}", i, SDL_GetVideoDriver(i));
//   }
//   auto sdl_window_result = SDL_Init(SDL_INIT_VIDEO);
//   debug::debug_print("sdl_window_result: {}", sdl_window_result);
//   if (!sdl_window_result) {
//     debug::debug_print("Failed to initialize SDL");
//     return;
//   }
//   if (!SDL_Vulkan_LoadLibrary(nullptr)) {
//     debug::debug_print("Failed to load Vulkan library");
//     return;
//   };
//   SDL_Window *window = SDL_CreateWindow(
//       "Gravwll + Vulkan + SDL3", 800, 600,
//       SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
//
//   debug::debug_print("Текущий видео драйвер: {}",
//   SDL_GetCurrentVideoDriver()); if (!window) {
//     debug::debug_print("Failed to create window: {}", SDL_GetError());
//     SDL_Quit();
//     return;
//   }
//
//   vkb::InstanceBuilder instance_builder;
//   auto instance_ret = instance_builder.set_app_name(AppName.c_str())
//                           .request_validation_layers(true)
//                           .use_default_debug_messenger()
//                           .build();
//   if (!instance_ret) {
//     debug::debug_print("Failed to create vulkan instance: {}",
//                        instance_ret.error().message());
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//     return;
//   }
//   vkb::Instance vkb_instance = instance_ret.value();
//   debug::debug_print("Created vkb_instance. This is api version: {}",
//                      vkb_instance.instance_version);
//
//   VkSurfaceKHR surface = VK_NULL_HANDLE;
//   if (!SDL_Vulkan_CreateSurface(window, vkb_instance.instance, nullptr,
//                                 &surface)) {
//     debug::debug_print("Failed to create Vulkan surface: {}",
//     SDL_GetError()); vkb::destroy_instance(vkb_instance);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//     return;
//   }
//   debug::debug_print("Created Vulkan surface");
//
//   vkb::PhysicalDeviceSelector selector{vkb_instance};
//   auto physical_device_ret = selector.set_surface(surface).select();
//   if (!physical_device_ret) {
//     debug::debug_print("Failed to select physical_device_ret: {}",
//                        physical_device_ret.error().message());
//     vkDestroySurfaceKHR(vkb_instance.instance, surface, nullptr);
//     vkb::destroy_instance(vkb_instance);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//   } else {
//     debug::debug_print("Created PhysicalDevice. Has value: {}",
//                        physical_device_ret.has_value());
//   }
//
//   vkb::PhysicalDevice physical_device = physical_device_ret.value();
//   vkb::DeviceBuilder device_builder{physical_device};
//   auto device_ret = device_builder.build();
//
//   if (!device_ret) {
//     debug::debug_print("Failed to create logical device: {}",
//                        device_ret.error().message());
//     vkDestroySurfaceKHR(vkb_instance.instance, surface, nullptr);
//     vkb::destroy_instance(vkb_instance);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//     return;
//   } else {
//     debug::debug_print("Create logical device: {}", device_ret.has_value());
//   }
//
//   vkb::Device vkb_device = device_ret.value();
//   VkQueue graphics_queue =
//       vkb_device.get_queue(vkb::QueueType::graphics).value();
//   uint32_t graphics_queue_family_index =
//       vkb_device.get_queue_index(vkb::QueueType::graphics).value();
//
//   debug::debug_print("Graphics queue: {}, Queue family index: {}",
//                      (void *)graphics_queue, graphics_queue_family_index);
//   std::cout << "Test" << std::endl;
//   debug::debug_print("Line 128");
//   vkb::SwapchainBuilder swapchain_builder{vkb_device};
//   debug::debug_print("swapchain_builder inited");
//   auto swapchain_ret = swapchain_builder.build();
//
//   debug::debug_print("swapchain_ret builded");
//   if (!swapchain_ret) {
//     debug::debug_print("Failed create swapchain: {}",
//                        swapchain_ret.error().message());
//     vkb::destroy_device(vkb_device);
//     vkDestroySurfaceKHR(vkb_instance.instance, surface, nullptr);
//     vkb::destroy_instance(vkb_instance);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//     return;
//   } else {
//     debug::debug_print("We craeted swapchain");
//   }
//   SDL_ShowWindow(window);
//
//   vkb::Swapchain vkb_swapchain = swapchain_ret.value();
//   std::cout << "QQQQQQQQQQQQQ"
//             << "Vulkan + SDL + vkb + VMA successfully inited!" << std::endl;
//
//   bool running = false;
//   while (running) {
// SDL_Event event;
// while (SDL_PollEvent(&event)) {
// if (event.type == SDL_EVENT_QUIT) {
// debug::debug_print("The quit was called. Shutting down");
// running = false;
// break;
// }
// }
//   }
//   return;
//   vkb::destroy_swapchain(vkb_swapchain);
//   vkb::destroy_device(vkb_device);
//   vkDestroySurfaceKHR(vkb_instance.instance, surface, nullptr);
//   vkb::destroy_instance(vkb_instance);
//   SDL_DestroyWindow(window);
//   SDL_Quit();
//   debug::debug_print("Vulkan + SDL + vkb + VMA successfully cleaned!");
// }
//
// class SDLException final : public std::runtime_error {
// public:
//   explicit SDLException(const std::string &message)
//       : std::runtime_error(std::format("{}: {}", message, SDL_GetError())) {}
// };
//
// constexpr auto VULKAN_VERSION{vk::makeApiVersion(0, 1, 4, 0)};
//
// struct Frame {
//   vk::raii::CommandBuffer commandBuffer;
//   vk::raii::Semaphore imageAvailableSemaphore;
//   vk::raii::Semaphore renderFinishedSemaphore;
//   vk::raii::Fence fence;
// };
//
// constexpr uint32_t IN_FLIGHT_FRAME_COUNT{2};
//
// class App {
//   std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window{
//       nullptr, SDL_DestroyWindow};
//   bool running{true};
//
//   std::optional<vk::raii::Context> context{};
//   std::optional<vk::raii::Instance> instance{};
//   std::optional<vk::raii::SurfaceKHR> surface{};
//   std::optional<vk::raii::PhysicalDevice> physicalDevice{};
//   uint32_t graphicsQueueFamilyIndex{};
//   std::optional<vk::raii::Device> device{};
//   std::optional<vk::raii::Queue> graphicsQueue{};
//
//   std::optional<vk::raii::CommandPool> commandPool{};
//   std::array<std::optional<Frame>, IN_FLIGHT_FRAME_COUNT> frames{};
//   uint32_t frameIndex{};
//
//   std::optional<vk::raii::SwapchainKHR> swapchain{};
//   std::vector<vk::Image> swapchainImages{};
//   vk::Extent2D swapchainExtent{};
//   vk::Format swapchainImageFormat{vk::Format::eB8G8R8A8Srgb};
//   uint32_t currentSwapchainImageIndex{};
//
// public:
//   App() {
//     if (!SDL_Init(SDL_INIT_VIDEO))
//       throw SDLException("Failed to initialize SDL");
//     if (!SDL_Vulkan_LoadLibrary(nullptr))
//       throw SDLException("Failed to load Vulkan library");
//     window.reset(SDL_CreateWindow("Codotaku", 800, 600,
//                                   SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
//                                       SDL_WINDOW_HIDDEN));
//     if (!window)
//       throw SDLException("Failed to create window");
//     auto vkGetInstanceProcAddr{reinterpret_cast<PFN_vkGetInstanceProcAddr>(
//         SDL_Vulkan_GetVkGetInstanceProcAddr())};
//     context.emplace(vkGetInstanceProcAddr);
//     auto const vulkanVersion{context->enumerateInstanceVersion()};
//     debug::debug_print("Vulkan {}.{}", VK_API_VERSION_MAJOR(vulkanVersion),
//                        VK_API_VERSION_MINOR(vulkanVersion));
//   }
//
//   ~App() {
//     device->waitIdle();
//     SDL_Quit();
//   }
//
//   void Init() {
//     InitInstance();
//     InitSurface();
//     PickPhysicalDevice();
//     InitDevice();
//     InitCommandPool();
//     InitFrames();
//     RecreateSwapchain();
//   }
//
//   void Run() {
//     SDL_ShowWindow(window.get());
//     while (running) {
//       HandleEvents();
//       Render();
//     }
//   }
//
// private:
//   struct ImageLayout {
//     vk::ImageLayout imageLayout{};
//     vk::PipelineStageFlags2 stageMask{};
//     vk::AccessFlags2 accessMask{};
//     uint32_t queueFamilyIndex{VK_QUEUE_FAMILY_IGNORED};
//   };
//
//   static void
//   TransitionImageLayout(vk::raii::CommandBuffer const &commandBuffer,
//                         vk::Image const &image, ImageLayout const &oldLayout,
//                         ImageLayout const &newLayout) {
//     vk::ImageMemoryBarrier2 const barrier{
//         oldLayout.stageMask,
//         oldLayout.accessMask,
//         newLayout.stageMask,
//         newLayout.accessMask,
//         oldLayout.imageLayout,
//         newLayout.imageLayout,
//         oldLayout.queueFamilyIndex,
//         newLayout.queueFamilyIndex,
//         image,
//         vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0,
//         1}};
//     vk::DependencyInfo dependencyInfo{};
//     dependencyInfo.setImageMemoryBarriers(barrier);
//     commandBuffer.pipelineBarrier2(dependencyInfo);
//   }
//
//   static void RecordCommandBuffer(vk::raii::CommandBuffer const
//   &commandBuffer,
//                                   vk::Image const &swapchainImage) {
//     commandBuffer.reset();
//     vk::CommandBufferBeginInfo beginInfo{};
//     beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
//     commandBuffer.begin(beginInfo);
//
//     auto const t{static_cast<double>(SDL_GetTicks()) * 0.001};
//
//     vk::ClearColorValue const color{std::array{
//         static_cast<float>(std::sin(t * 5.0) * 0.5 + 0.5), 0.0f,
//         0.0f, 1.0f}};
//
//     TransitionImageLayout(commandBuffer, swapchainImage,
//                           ImageLayout{
//                               vk::ImageLayout::eUndefined,
//                               vk::PipelineStageFlagBits2::eTransfer,
//                               vk::AccessFlagBits2KHR::eMemoryRead,
//                           },
//                           ImageLayout{
//                               vk::ImageLayout::eTransferDstOptimal,
//                               vk::PipelineStageFlagBits2::eTransfer,
//                               vk::AccessFlagBits2KHR::eTransferWrite,
//                           });
//
//     commandBuffer.clearColorImage(
//         swapchainImage, vk::ImageLayout::eTransferDstOptimal, color,
//         vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0,
//         1});
//
//     TransitionImageLayout(commandBuffer, swapchainImage,
//                           ImageLayout{
//                               vk::ImageLayout::eTransferDstOptimal,
//                               vk::PipelineStageFlagBits2::eTransfer,
//                               vk::AccessFlagBits2KHR::eTransferWrite,
//                           },
//                           ImageLayout{
//                               vk::ImageLayout::ePresentSrcKHR,
//                               vk::PipelineStageFlagBits2::eTransfer,
//                               vk::AccessFlagBits2KHR::eMemoryRead,
//                           });
//
//     commandBuffer.end();
//   }
//
//   void BeginFrame(Frame const &frame) {
//     auto _ = device->waitForFences(*frame.fence, VK_TRUE, UINT64_MAX);
//     device->resetFences(*frame.fence);
//
//     auto [acquireResult, imageIndex] = swapchain->acquireNextImage(
//         UINT64_MAX, *frame.imageAvailableSemaphore, nullptr);
//     currentSwapchainImageIndex = imageIndex;
//   }
//
//   void EndFrame(Frame const &frame) {
//     vk::PresentInfoKHR presentInfo{};
//     presentInfo.setSwapchains(**swapchain);
//     presentInfo.setImageIndices(currentSwapchainImageIndex);
//     presentInfo.setWaitSemaphores(*frame.renderFinishedSemaphore);
//     auto _ = graphicsQueue->presentKHR(presentInfo);
//
//     frameIndex = (frameIndex + 1) % IN_FLIGHT_FRAME_COUNT;
//   }
//
//   void SubmitCommandBuffer(Frame const &frame) const {
//     vk::SubmitInfo submitInfo{};
//     submitInfo.setCommandBuffers(*frame.commandBuffer);
//     submitInfo.setWaitSemaphores(*frame.imageAvailableSemaphore);
//     submitInfo.setSignalSemaphores(*frame.renderFinishedSemaphore);
//     constexpr vk::PipelineStageFlags waitStage{
//         vk::PipelineStageFlagBits::eTransfer};
//     submitInfo.setWaitDstStageMask(waitStage);
//     graphicsQueue->submit(submitInfo, frame.fence);
//   }
//
//   void Render() {
//     auto const &frame{*frames[frameIndex]};
//     BeginFrame(frame);
//     RecordCommandBuffer(frame.commandBuffer,
//                         swapchainImages[currentSwapchainImageIndex]);
//     SubmitCommandBuffer(frame);
//     EndFrame(frame);
//   }
//
// void HandleEvents() {
//   for (SDL_Event event; SDL_PollEvent(&event);)
//     switch (event.type) {
//     case SDL_EVENT_QUIT:
//       running = false;
//       break;
//     case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
//       RecreateSwapchain();
//       break;
//     default:
//       break;
//     }
// }
//
//   void RecreateSwapchain() {
//     vk::SurfaceCapabilitiesKHR const surfaceCapabilities{
//         physicalDevice->getSurfaceCapabilitiesKHR(*surface)};
//     swapchainExtent = surfaceCapabilities.currentExtent;
//
//     vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
//     swapchainCreateInfo.surface = *surface;
//     swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount +
//     1; swapchainCreateInfo.imageFormat = swapchainImageFormat;
//     swapchainCreateInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
//     swapchainCreateInfo.imageExtent = swapchainExtent;
//     swapchainCreateInfo.imageArrayLayers = 1;
//     swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment
//     |
//                                      vk::ImageUsageFlagBits::eTransferDst;
//     swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
//     swapchainCreateInfo.compositeAlpha =
//     vk::CompositeAlphaFlagBitsKHR::eOpaque; swapchainCreateInfo.presentMode =
//     vk::PresentModeKHR::eMailbox; swapchainCreateInfo.clipped = true;
//
//     // todo: figure out why old swapchain handle is invalid
//     // if (swapchain.has_value()) swapchainCreateInfo.oldSwapchain =
//     // **swapchain;
//
//     swapchain.emplace(*device, swapchainCreateInfo);
//     swapchainImages = swapchain->getImages();
//   }
//
//   void InitFrames() {
//     vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
//     commandBufferAllocateInfo.commandPool = *commandPool;
//     commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
//     commandBufferAllocateInfo.commandBufferCount = IN_FLIGHT_FRAME_COUNT;
//     auto commandBuffers{
//         device->allocateCommandBuffers(commandBufferAllocateInfo)};
//
//     for (size_t i = 0; i < IN_FLIGHT_FRAME_COUNT; i++)
//       frames[i].emplace(
//           std::move(commandBuffers[i]),
//           vk::raii::Semaphore{*device, vk::SemaphoreCreateInfo{}},
//           vk::raii::Semaphore{*device, vk::SemaphoreCreateInfo{}},
//           vk::raii::Fence{*device, vk::FenceCreateInfo{
//                                        vk::FenceCreateFlagBits::eSignaled}});
//   }
//
//   void InitCommandPool() {
//     vk::CommandPoolCreateInfo commandPoolCreateInfo{};
//     commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
//     commandPoolCreateInfo.flags =
//         vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
//
//     commandPool.emplace(*device, commandPoolCreateInfo);
//   }
//
//   void InitDevice() {
//     vk::DeviceQueueCreateInfo queueCreateInfo{};
//     queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
//     std::array queuePriorities{1.0f};
//     queueCreateInfo.setQueuePriorities(queuePriorities);
//
//     vk::DeviceCreateInfo deviceCreateInfo{};
//     std::array queueCreateInfos{queueCreateInfo};
//     deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
//     std::array<const char *const, 1> enabledExtensions{
//         VK_KHR_SWAPCHAIN_EXTENSION_NAME};
//     deviceCreateInfo.setPEnabledExtensionNames(enabledExtensions);
//
//     vk::PhysicalDeviceVulkan13Features vulkan13Features{};
//     vulkan13Features.synchronization2 = true;
//
//     vk::StructureChain chain{deviceCreateInfo, vulkan13Features};
//
//     device.emplace(*physicalDevice, chain.get<vk::DeviceCreateInfo>());
//
//     graphicsQueue.emplace(*device, graphicsQueueFamilyIndex, 0);
//   }
//
//   void PickPhysicalDevice() {
//     auto const physicalDevices{instance->enumeratePhysicalDevices()};
//     if (physicalDevices.empty())
//       throw std::runtime_error("No Vulkan devices found");
//     physicalDevice.emplace(*instance, *physicalDevices.front());
//     auto const rawDeviceName{physicalDevice->getProperties().deviceName};
//     std::string deviceName(rawDeviceName.data(), std::strlen(rawDeviceName));
//     debug::debug_print("{}", deviceName);
//     graphicsQueueFamilyIndex = 0;
//   }
//
//   void InitInstance() {
//     vk::ApplicationInfo applicationInfo{};
//     applicationInfo.apiVersion = VULKAN_VERSION;
//
//     vk::InstanceCreateInfo instanceCreateInfo{};
//     instanceCreateInfo.pApplicationInfo = &applicationInfo;
//     uint32_t extensionCount;
//     instanceCreateInfo.ppEnabledExtensionNames =
//         SDL_Vulkan_GetInstanceExtensions(&extensionCount);
//     instanceCreateInfo.enabledExtensionCount = extensionCount;
//
//     instance.emplace(*context, instanceCreateInfo);
//   }
//
//   void InitSurface() {
//     VkSurfaceKHR raw_surface;
//     if (!SDL_Vulkan_CreateSurface(window.get(), **instance, nullptr,
//                                   &raw_surface))
//       throw SDLException("Failed to create Vulkan surface");
//     surface.emplace(*instance, raw_surface);
//   }
// };
//
// void GfxEngine::init_vulkan() {
//   try {
//     App app{};
//     debug::debug_print("Текущий видео драйвер: {}",
//                        SDL_GetCurrentVideoDriver());
//     debug::debug_print("This is a breakpoint#1");
//     app.Init();
//     debug::debug_print("This is a breakpoint#2");
//     app.Run();
//   } catch (const SDLException &e) {
//     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: %s", e.what());
//     SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", e.what(),
//     nullptr); return;
//   }
//   return;
// }
