#include "gfx/renderer/renderer.h"
#include "gfx/vulkan_core.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/trigonometric.hpp>
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

void Renderer::update_uniform_buffer(vulkan_core::Frame &frame,
                                     float delta_time) {
  static float total_time = 0.0f;
  total_time += 0.016f;

  static vulkan_core::CameraUBO ubo{total_time};

  // 1. Базовые параметры
  ubo.point_size = 4.0f;
  ubo.time = total_time;

  // 2. Позиция камеры (вращается вокруг центра)
  float radius = 5.0f;
  float angle = total_time * 0.1f;
  ubo.camera_pos =
      glm::vec4(radius * sin(angle) - total_time * total_time,
                radius * cos(angle) * 0.3f, radius * cos(angle) * 0.3f, 1.0f);

  // 3. Видовая матрица
  ubo.view = glm::lookAt(glm::vec3(ubo.camera_pos), glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(0.0f, 0.0f, 1.0f));

  // 4. Перспективная проекция
  float aspect = primitives.swapchainExtent.width /
                 (float)primitives.swapchainExtent.height;
  ubo.projection = glm::perspective(glm::radians(60.0f), // поле зрения
                                    aspect,              // соотношение сторон
                                    0.1f,                // ближняя плоскость
                                    1000.0f              // дальняя плоскость
  );

  // 5. Инвертируем Y для Vulkan
  ubo.projection[1][1] *= -1;
  debug::debug_print("Camera position: [{:.2f}, {:.2f}, {:.2f}]",
                     ubo.camera_pos.x, ubo.camera_pos.y, ubo.camera_pos.z);

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
  static auto last_time = std::chrono::high_resolution_clock::now();
  auto current_time = std::chrono::high_resolution_clock::now();
  float delta_time =
      std::chrono::duration<float>(current_time - last_time).count();
  last_time = current_time;

  update_uniform_buffer(frame, delta_time);

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
