#include "gfx/renderer/renderer.h"
#include "chrono"
#include "gfx/vulkan_core.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

namespace render {

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

  if (auto e = vulkan_core::create_uniform_buffers(primitives); e.is_error())
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

  std::cout << "Step 10/10. Renderer is constructed!\n" << std::flush;
}

Renderer::~Renderer() {
  try {
    debug::debug_print("Destroying Renderer");
    primitives.clean_up();
    debug::debug_print("Renderer destroyed");
  } catch (const vk::SystemError &e) {
    debug::debug_print("Error in Renderer destructor: {}", e.what());
  }
}

void Renderer::update_uniform_buffer(vulkan_core::Frame &frame) {
  static auto startTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

  vulkan_core::UniformBufferObject ubo{};

  ubo.mvp = glm::mat4(1.0f); // Identity matrix - no transformation

  // Для отладки: можно вывести матрицу
  debug::debug_print("Matrix mvp[0][0]: {}", ubo.mvp[0][0]);

  if (frame.uniform_buffer_mapped) {
    memcpy(frame.uniform_buffer_mapped, &ubo, sizeof(ubo));
  } else {
    debug::debug_print("ERROR: uniform_buffer_mapped is null!");
  }
}

error::Result<bool> Renderer::render_frame() {
  auto &frame = primitives.frames[primitives.frameIndex];

  debug::debug_print("Rendering frame {}, image index: {}",
                     primitives.frameIndex,
                     primitives.currentSwapchainImageIndex);
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
