#include "gfx/renderer/renderer.h"
#include "gfx/vulkan_core.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <cmath>
#include <iostream>
#include <vulkan/vulkan_core.h>

namespace render {
error::Result<bool> Renderer::begin_frame(const vulkan_core::Frame &fr) {
  auto _ = primitives.device.waitForFences(*fr.fence, VK_TRUE, UINT64_MAX);
  primitives.device.resetFences(*fr.fence);

  auto [acquireResult, imageIndex] = primitives.swapchain.acquireNextImage(
      UINT64_MAX, *fr.imageAvailableSemaphore, nullptr);
  primitives.currentSwapchainImageIndex = imageIndex;
  return error::Result<bool>::success(true);
}

static void TransitionImageLayout(vk::raii::CommandBuffer const &commandBuffer,
                                  vk::Image const &image,
                                  vulkan_core::ImageLayout const &oldLayout,
                                  vulkan_core::ImageLayout const &newLayout) {
  vk::ImageMemoryBarrier2 const barrier{
      oldLayout.stageMask,
      oldLayout.accessMask,
      newLayout.stageMask,
      newLayout.accessMask,
      oldLayout.imageLayout,
      newLayout.imageLayout,
      oldLayout.queueFamilyIndex,
      newLayout.queueFamilyIndex,
      image,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
  vk::DependencyInfo dependencyInfo{};
  dependencyInfo.setImageMemoryBarriers(barrier);
  commandBuffer.pipelineBarrier2(dependencyInfo);
}

static void RecordCommandBuffer(vk::raii::CommandBuffer const &commandBuffer,
                                vk::Image const &swapchainImage) {
  commandBuffer.reset();
  vk::CommandBufferBeginInfo beginInfo{};
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  commandBuffer.begin(beginInfo);

  auto const t{static_cast<double>(SDL_GetTicks()) * 0.001};

  vk::ClearColorValue const color{std::array{
      static_cast<float>(std::sin(t * 5.0) * 0.5 + 0.5), 0.0f, 0.0f, 1.0f}};
  TransitionImageLayout(commandBuffer, swapchainImage,
                        vulkan_core::ImageLayout{
                            vk::ImageLayout::eUndefined,
                            vk::PipelineStageFlagBits2::eTransfer,
                            vk::AccessFlagBits2KHR::eMemoryRead,
                        },
                        vulkan_core::ImageLayout{
                            vk::ImageLayout::eTransferDstOptimal,
                            vk::PipelineStageFlagBits2::eTransfer,
                            vk::AccessFlagBits2KHR::eTransferWrite,
                        });

  commandBuffer.clearColorImage(
      swapchainImage, vk::ImageLayout::eTransferDstOptimal, color,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
  TransitionImageLayout(commandBuffer, swapchainImage,
                        vulkan_core::ImageLayout{
                            vk::ImageLayout::eTransferDstOptimal,
                            vk::PipelineStageFlagBits2::eTransfer,
                            vk::AccessFlagBits2KHR::eTransferWrite,
                        },
                        vulkan_core::ImageLayout{
                            vk::ImageLayout::ePresentSrcKHR,
                            vk::PipelineStageFlagBits2::eTransfer,
                            vk::AccessFlagBits2KHR::eMemoryRead,
                        });
  commandBuffer.end();
}

void Renderer::EndFrame(vulkan_core::Frame const &frame) {
  vk::PresentInfoKHR presentInfo{};
  presentInfo.setSwapchains(*primitives.swapchain);
  presentInfo.setImageIndices(primitives.currentSwapchainImageIndex);
  presentInfo.setWaitSemaphores(*frame.renderFinishedSemaphore);
  auto _ = primitives.graphicsQueue.presentKHR(presentInfo);

  primitives.frameIndex =
      (primitives.frameIndex + 1) % vulkan_core::IN_FLIGHT_FRAME_COUNT;
}

void Renderer::SubmitCommandBuffer(vulkan_core::Frame const &frame) {
  vk::SubmitInfo submitInfo{};
  submitInfo.setCommandBuffers(*frame.commandBuffer);
  submitInfo.setWaitSemaphores(*frame.imageAvailableSemaphore);
  submitInfo.setSignalSemaphores(*frame.renderFinishedSemaphore);
  constexpr vk::PipelineStageFlags waitStage{
      vk::PipelineStageFlagBits::eTransfer};
  submitInfo.setWaitDstStageMask(waitStage);
  primitives.graphicsQueue.submit(submitInfo, frame.fence);
}

Renderer::Renderer(window::MyWindow &window)
    : primitives(vulkan_core::create_vulkan_core(window.window_name, window)
                     .unwrap()) {
  std::cout << "Renderer is inited!" << std::endl;
  vulkan_core::init_swapchain(primitives, window);
  vulkan_core::init_frames(primitives);
}

error::Result<bool> Renderer::render() {
  std::cout << "We are in Renderer::render, ready to aquire frame"
            << primitives.frameIndex << std::endl;
  auto const &frame(primitives.frames[primitives.frameIndex]);

  std::cout << "Step1" << std::endl;
  auto frame_start = begin_frame(frame);

  std::cout << "Step2" << std::endl;
  RecordCommandBuffer(
      frame.commandBuffer,
      primitives.swapchainImages[primitives.currentSwapchainImageIndex]);

  std::cout << "Step3" << std::endl;
  SubmitCommandBuffer(frame);
  std::cout << "Step4" << std::endl;
  // return error::Result<bool>::error(1, "");
  return error::Result<bool>::success(true);
}

int Renderer::handle_events() {
  for (SDL_Event event; SDL_PollEvent(&event);)
    switch (event.type) {
    case SDL_EVENT_QUIT:
      return 1;
      // case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      // RecreateSwapchain();
      break;
    default:
      break;
    }
  return 0;
}

} // namespace render
