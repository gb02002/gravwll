#include "gfx/renderer/particle_renderer.h"
#include "gfx/core/input.h"
#include "gfx/renderer/uniform_data.h"
#include "gfx/vulkan_core/commands.h"
#include "gfx/vulkan_core/device.h"
#include "gfx/vulkan_core/pipeline.h"
#include "gfx/vulkan_core/swapchain.h"
#include "utils/namespaces/error_namespace.h"
#include <algorithm>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <string>

namespace gfx::renderer {

ParticleRenderer::ParticleRenderer(window::MyWindow &window)
    : window_(window), input_manager_(window_),
      camera_controller_(scene_.get_camera(), input_manager_) {}

ParticleRenderer::~ParticleRenderer() {
  try {
    if (initialized_) {
      debug::debug_print("Destroying ParticleRenderer");
      vulkan_core_.clean_up();
      debug::debug_print("ParticleRenderer destroyed");
    }
  } catch (const vk::SystemError &e) {
    debug::debug_print("Error in ParticleRenderer destructor: {}", e.what());
  }
}

error::Result<bool> ParticleRenderer::init() {
  if (initialized_) {
    return error::Result<bool>::success(true);
  }

  try {
    debug::debug_print("Initializing ParticleRenderer");

    // 1. Создаем Vulkan core
    if (auto e = vulkan_core::create_vulkan_core(window_.window_name, window_);
        e.is_error()) {
      return error::Result<bool>::error(-1, e.error_message());
    } else {
      vulkan_core_ = e.unwrap();
    }

    // 2. Инициализация свопчейна
    if (auto e = vulkan_core::init_swapchain(vulkan_core_, window_);
        e.is_error())
      return e;

    // 3. Создаем image views
    if (auto e = vulkan_core::create_image_views(vulkan_core_); e.is_error())
      return e;

    // 4. Создаем render pass
    debug::debug_print("Creating render pass...");
    if (auto e = vulkan_core::create_render_pass(vulkan_core_); e.is_error())
      return e;

    // 5. Создаем графический пайплайн
    debug::debug_print("Creating graphics pipeline...");
    if (auto e = vulkan_core::create_graphics_pipeline(vulkan_core_);
        e.is_error())
      return e;

    // 6. Создаем framebuffers
    debug::debug_print("Creating framebuffers...");
    if (auto e = vulkan_core::create_framebuffers(vulkan_core_); e.is_error())
      return e;

    // 7. Создаем uniform буферы
    if (auto e = vulkan_core::create_uniform_buffers(vulkan_core_);
        e.is_error())
      return e;

    // 8. Создаем вершинный буфер
    debug::debug_print("Creating vertex buffer");
    if (auto e = vulkan_core::create_vertex_buffer(vulkan_core_); e.is_error())
      return e;

    // 9. Создаем пул дескрипторов и наборы
    debug::debug_print("Creating descriptor pool and sets");
    if (auto e = vulkan_core::create_descriptor_pool_and_sets(vulkan_core_);
        e.is_error())
      return e;

    // 10. Инициализируем кадры (создаем командные буферы и синхронизацию)
    debug::debug_print("Creating synchronizing frames");
    vulkan_core::init_frames(vulkan_core_);

    // 11. Настраиваем начальное состояние камеры
    scene_.get_camera().set_perspective(
        60.0f,
        static_cast<float>(vulkan_core_.swapchainExtent.width) /
            vulkan_core_.swapchainExtent.height,
        0.1f, 1000.0f);
    scene_.get_camera().look_at(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));

    initialized_ = true;
    debug::debug_print("ParticleRenderer initialized successfully");

    return error::Result<bool>::success(true);

  } catch (const std::exception &e) {
    return error::Result<bool>::error(
        -1, std::string(std::string("Failed to initialize ParticleRenderer: ") +
                        e.what())
                .c_str());
  }
}

error::Result<bool> ParticleRenderer::update(float delta_time) {
  delta_time_ = delta_time;
  total_time_ += delta_time;

  // Обновляем контроллер камеры
  camera_controller_.update(delta_time);

  // Обновляем сцену
  scene_.update(delta_time);
  if (input_manager_.get_state().mouse.wheel != 0.0f) {
    point_size_ += input_manager_.get_state().mouse.wheel * 0.1f;
    point_size_ = std::max(1.0f, std::min(point_size_, 32.0f));
  }
  return error::Result<bool>::success(true);
}

error::Result<bool> ParticleRenderer::render_frame() {
  if (!initialized_) {
    return error::Result<bool>::error(-1, "ParticleRenderer not initialized");
  }

  auto &frame = vulkan_core_.frames[vulkan_core_.frameIndex];

  auto wait_result =
      vulkan_core_.device.waitForFences(*frame.fence, VK_TRUE, UINT64_MAX);
  if (wait_result != vk::Result::eSuccess) {
    return error::Result<bool>::error(-1, "Failed to wait for fence");
  }
  vulkan_core_.device.resetFences(*frame.fence);

  uint32_t image_index;
  if (auto e = acquire_swapchain_image(frame, image_index); e.is_error()) {
    return e;
  }

  vulkan_core_.currentSwapchainImageIndex = image_index;

  if (auto e = update_uniform_buffers(delta_time_); e.is_error()) {
    return e;
  }

  frame.commandBuffer.reset();
  if (auto e = vulkan_core::record_frame_command_buffer(vulkan_core_, frame,
                                                        image_index);
      e.is_error()) {
    return e;
  }

  if (auto e = submit_commands(frame); e.is_error()) {
    return e;
  }

  if (auto e = present_frame(frame, image_index); e.is_error()) {
    return e;
  }

  vulkan_core_.frameIndex =
      (vulkan_core_.frameIndex + 1) % vulkan_core::IN_FLIGHT_FRAME_COUNT;

  return error::Result<bool>::success(true);
}

error::Result<bool> ParticleRenderer::update_uniform_buffers(float delta_time) {
  auto &frame = vulkan_core_.frames[vulkan_core_.frameIndex];

  // Получаем камеру из сцены
  auto &camera = scene_.get_camera();

  // Создаем UBO
  RenderUniform ubo;
  ubo.view = camera.get_view_matrix();

  // Создаем проекционную матрицу
  float aspect = static_cast<float>(vulkan_core_.swapchainExtent.width) /
                 vulkan_core_.swapchainExtent.height;
  ubo.projection = glm::perspective(
      glm::radians(camera.get_fov() / zoom_level_), aspect, 0.1f, 1000.0f);
  ubo.projection[1][1] *= -1; // Инвертируем Y для Vulkan

  ubo.camera_pos = glm::vec4(camera.get_position(), 1.0f);
  ubo.point_size = point_size_;
  ubo.time = total_time_;
  ubo.zoom_level = zoom_level_;
  ubo.brightness = brightness_;

  // Копируем в буфер
  if (frame.uniform_buffer_mapped) {
    memcpy(frame.uniform_buffer_mapped, &ubo, sizeof(ubo));
    return error::Result<bool>::success(true);
  } else {
    return error::Result<bool>::error(-1, "Uniform buffer not mapped");
  }
}

error::Result<bool>
ParticleRenderer::acquire_swapchain_image(vulkan_core::Frame &frame,
                                          uint32_t &image_index) {
  auto [acquire_result, index] = vulkan_core_.swapchain.acquireNextImage(
      UINT64_MAX, *frame.imageAvailableSemaphore, nullptr);
  image_index = index;

  if (acquire_result != vk::Result::eSuccess &&
      acquire_result != vk::Result::eSuboptimalKHR) {
    return error::Result<bool>::error(-1, "Failed to acquire swapchain image");
  }

  return error::Result<bool>::success(true);
}

error::Result<bool>
ParticleRenderer::submit_commands(vulkan_core::Frame &frame) {
  vk::SubmitInfo submit_info{};
  vk::Semaphore wait_semaphores[] = {*frame.imageAvailableSemaphore};
  vk::PipelineStageFlags wait_stages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &*frame.commandBuffer;

  vk::Semaphore signal_semaphores[] = {*frame.renderFinishedSemaphore};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  vulkan_core_.graphicsQueue.submit(submit_info, *frame.fence);

  return error::Result<bool>::success(true);
}

error::Result<bool> ParticleRenderer::present_frame(vulkan_core::Frame &frame,
                                                    uint32_t image_index) {
  vk::PresentInfoKHR present_info{};
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &*frame.renderFinishedSemaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &*vulkan_core_.swapchain;
  present_info.pImageIndices = &image_index;

  auto present_result = vulkan_core_.graphicsQueue.presentKHR(present_info);

  if (present_result != vk::Result::eSuccess &&
      present_result != vk::Result::eSuboptimalKHR) {
    return error::Result<bool>::error(-1, "Failed to present swapchain image");
  }

  return error::Result<bool>::success(true);
}

} // namespace gfx::renderer
