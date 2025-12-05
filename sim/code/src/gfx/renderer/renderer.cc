#include "gfx/renderer/renderer.h"
#include "chrono"
#include "gfx/vulkan_core.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
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

// Объявим функции, которые используются в render()
void Renderer::record_draw_commands(vulkan_core::Frame &frame,
                                    uint32_t imageIndex) {
  // TODO: реализовать запись команд отрисовки
  // Пока заглушка
}

void Renderer::submit_commands(const vulkan_core::Frame &frame) {
  // TODO: реализовать отправку команд
  // Пока используем существующую функцию (если есть) или создадим
}

void Renderer::present_frame(const vulkan_core::Frame &frame) {
  // TODO: реализовать отображение кадра
  // Пока используем существующую EndFrame
}

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

  debug::debug_print("Step 0/10. We are in Renderer::Renderer");
  if (auto e = vulkan_core::init_swapchain(primitives, window); e.is_error())
    throw std::runtime_error(e.err_msg);

  if (auto e = vulkan_core::create_image_views(primitives); e.is_error())
    throw std::runtime_error(e.err_msg);

  debug::debug_print("Step 1/10. Creating render pass...");
  if (auto e = vulkan_core::create_render_pass(primitives); e.is_error())
    throw std::runtime_error(e.err_msg);

  debug::debug_print("Step 2/10. Creating graphics pipeline...");
  if (auto e = vulkan_core::create_graphics_pipeline(primitives); e.is_error())
    throw std::runtime_error(e.err_msg);

  debug::debug_print("Step 3/10. Creating framebuffers...");
  if (auto e = vulkan_core::create_framebuffers(primitives); e.is_error())
    throw std::runtime_error(e.err_msg);

  // debug::debug_print("Step 4/10. Creating command buffers...");
  // if (auto e = vulkan_core::create_command_buffers(primitives); e.is_error())
  // throw std::runtime_error(e.err_msg);

  debug::debug_print("Step 6/11. Creating vertex buffer");
  if (auto e = vulkan_core::create_vertex_buffer(primitives); e.is_error())
    throw std::runtime_error(e.err_msg);

  debug::debug_print("Step 7/10. Creating descriptor pool and sets");
  if (auto e = vulkan_core::create_descriptor_pool_and_sets(primitives);
      e.is_error())
    throw std::runtime_error(e.err_msg);

  // Синхронизация (ваши frames) - создаст command buffers в каждом Frame
  debug::debug_print("Step 9/10. Creating synchronizing frames");
  vulkan_core::init_frames(primitives);

  debug::debug_print("Step 10/10. Renderer is constructed!");
}

void Renderer::update_uniform_buffer(vulkan_core::Frame &frame) {
  static auto startTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

  vulkan_core::UniformBufferObject ubo{};

  // Создаем простую вращающуюся матрицу
  glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat4 view =
      glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat4 proj =
      glm::perspective(glm::radians(45.0f),
                       primitives.swapchainExtent.width /
                           (float)primitives.swapchainExtent.height,
                       0.1f, 10.0f);
  proj[1][1] *= -1; // Инвертируем Y для Vulkan
  ubo.mvp = proj * view * model;

  // Копируем данные в uniform буфер (память уже отображена)
  memcpy(frame.uniform_buffer_mapped, &ubo, sizeof(ubo));
}

// error::Result<bool> Renderer::render() {
//   auto const &frame(primitives.frames[primitives.frameIndex]);
//
//   auto frame_start = begin_frame(frame);
//   if (frame_start.is_error()) {
//     debug::debug_print("begin_frame got {}", frame_start.err_msg);
//     error::Result<bool>::error(-1, "Can't begin frame");
//   }
//
//   RecordCommandBuffer(
//       frame.commandBuffer,
//       primitives.swapchainImages[primitives.currentSwapchainImageIndex]);
//
//   // FIX: Undefined
//   // SubmitCommandBuffer(frame);
//   EndFrame(frame);
//   // return error::Result<bool>::error(1, "");
//   return error::Result<bool>::success(true);
// }

error::Result<bool> Renderer::render_frame() {
  auto &frame = primitives.frames[primitives.frameIndex];

  // Ждем, пока кадр освободится
  auto waitResult =
      primitives.device.waitForFences(*frame.fence, VK_TRUE, UINT64_MAX);
  if (waitResult != vk::Result::eSuccess) {
    return error::Result<bool>::error(-1, "Failed to wait for fence");
  }
  primitives.device.resetFences(*frame.fence);

  // Получаем изображение из swapchain
  uint32_t imageIndex;
  auto [acquireResult, index] = primitives.swapchain.acquireNextImage(
      UINT64_MAX, *frame.imageAvailableSemaphore, nullptr);
  imageIndex = index;

  if (acquireResult != vk::Result::eSuccess &&
      acquireResult != vk::Result::eSuboptimalKHR) {
    return error::Result<bool>::error(-1, "Failed to acquire swapchain image");
  }

  primitives.currentSwapchainImageIndex = imageIndex;

  // Обновляем uniform буфер для текущего кадра
  update_uniform_buffer(frame);

  // Сбрасываем и записываем командный буфер текущего кадра
  frame.commandBuffer.reset();
  if (auto e = vulkan_core::record_frame_command_buffer(primitives, frame,
                                                        imageIndex);
      e.is_error()) {
    return e;
  }

  // Отправляем команды
  vk::SubmitInfo submitInfo{};
  vk::Semaphore waitSemaphores[] = {*frame.imageAvailableSemaphore};
  vk::PipelineStageFlags waitStages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &*frame.commandBuffer;

  vk::Semaphore signalSemaphores[] = {*frame.renderFinishedSemaphore};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  primitives.graphicsQueue.submit(submitInfo, *frame.fence);

  // Отображаем кадр
  vk::PresentInfoKHR presentInfo{};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &*primitives.swapchain;
  presentInfo.pImageIndices = &imageIndex;

  auto presentResult = primitives.graphicsQueue.presentKHR(presentInfo);

  if (presentResult != vk::Result::eSuccess &&
      presentResult != vk::Result::eSuboptimalKHR) {
    return error::Result<bool>::error(-1, "Failed to present swapchain image");
  }

  // Переходим к следующему кадру
  primitives.frameIndex =
      (primitives.frameIndex + 1) % vulkan_core::IN_FLIGHT_FRAME_COUNT;

  return error::Result<bool>::success(true);
}
} // namespace render
