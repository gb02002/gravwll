#include "gfx/renderer/renderer.h"
#include "gfx/vulkan_core.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <tuple>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

namespace render {

// acquireNextImage gets written to currentSwapchainImageIndex with error
// handling and awaiting fences
error::Result<bool> Renderer::begin_frame(const vulkan_core::Frame &fr) {
  auto _ = primitives.device.waitForFences(*fr.fence, VK_TRUE, UINT64_MAX);
  primitives.device.resetFences(*fr.fence);

  vk::Result result;
  uint32_t imageIndex;
  std::tie(result, imageIndex) = primitives.swapchain.acquireNextImage(
      UINT64_MAX, *fr.imageAvailableSemaphore, nullptr);
  if (result == vk::Result::eErrorOutOfDateKHR) {
    // swapchain устарел — нужно пересоздать swapchain (RecreateSwapchain)
    return error::Result<bool>::error(-1, "Swapchain out of date");
  } else if (result == vk::Result::eTimeout) {
    return error::Result<bool>::error(-1, "We got timeout");

  } else if (result != vk::Result::eSuccess &&
             result != vk::Result::eSuboptimalKHR) {
    return error::Result<bool>::error(-1, "Failed to acquire swapchain image");
  }
  assert(result == vk::Result::eSuccess);
  assert(imageIndex < primitives.swapchainImages.size());
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

  auto const t{static_cast<double>(SDL_GetTicks()) * 0.003};

  vk::ClearColorValue const color{std::array{
      static_cast<float>(std::sin(t * 5.0) * 0.5 + 0.5), 0.66f, 1.22f, 1.18f}};
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

Renderer::Renderer(window::MyWindow &window)
    : primitives(vulkan_core::create_vulkan_core(window.window_name, window)
                     .unwrap()) {

  vulkan_core::init_swapchain(primitives, window);
  vulkan_core::create_image_views(primitives);
  vulkan_core::create_render_pass(primitives);
  vulkan_core::create_graphics_pipeline(primitives);
  vulkan_core::create_framebuffers(primitives);

  if (auto e = vulkan_core::create_command_buffers(primitives); e.is_error()) {
    throw std::runtime_error(e.err_msg);
  }

  // TODO: Создать вершинные буферы здесь
  vulkan_core::create_vertex_buffer(primitives, particles);

  // TODO: Создать uniform буферы здесь
  vulkan_core::create_uniform_buffers(primitives);

  vulkan_core::create_descriptor_pool_and_sets(primitives);

  // Записать команды (после создания буферов!)
  if (auto e = vulkan_core::record_command_buffers(primitives); e.is_error()) {
    throw std::runtime_error(e.err_msg);
  }

  // Синхронизация (ваши frames)
  vulkan_core::init_frames(primitives);
}

error::Result<bool> Renderer::render() {
  auto const &frame(primitives.frames[primitives.frameIndex]);

  auto frame_start = begin_frame(frame);
  if (frame_start.is_error()) {
    debug::debug_print("begin_frame got {}", frame_start.err_msg);
    error::Result<bool>::error(-1, "Can't begin frame");
  }

  RecordCommandBuffer(
      frame.commandBuffer,
      primitives.swapchainImages[primitives.currentSwapchainImageIndex]);

  SubmitCommandBuffer(frame);
  EndFrame(frame);
  // return error::Result<bool>::error(1, "");
  return error::Result<bool>::success(true);
}

error::Result<bool> Renderer::render_frame() {
  auto &frame = primitives.frames[primitives.frameIndex];

  begin_frame(frame);
  update_uniform_buffer(frame);
  record_draw_commands(frame, primitives.currentSwapchainImageIndex);
  submit_commands(frame);
  present_frame(frame);
  primitives.frameIndex =
      (primitives.frameIndex + 1) % vulkan_core::IN_FLIGHT_FRAME_COUNT;
}

} // namespace render
