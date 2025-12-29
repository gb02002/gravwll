#include "gfx/renderer/particle_renderer.h"
#include "gfx/core/input.h"
#include "gfx/renderer/uniform_data.h"
#include "gfx/vulkan_core/buffer.h"
#include "gfx/vulkan_core/commands.h"
#include "gfx/vulkan_core/device.h"
#include "gfx/vulkan_core/pipeline.h"
#include "gfx/vulkan_core/swapchain.h"
#include "gfx/vulkan_core/types.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <string>
#include <vulkan/vulkan.hpp>

namespace gfx::renderer {

ParticleRenderer::ParticleRenderer(window::MyWindow &window)
    : window_(window), input_manager_(window_),
      camera_controller_(scene_.get_camera(), input_manager_) {}

ParticleRenderer::~ParticleRenderer() {
  try {
    if (initialized_) {
      debug::debug_print("Destroying ParticleRenderer");

      if (vulkan_core_.vertex_buffer_mapped) {
        vk::MemoryUnmapInfo unmap_info{};
        unmap_info.memory = *vulkan_core_.vertex_buffer_memory;
        vulkan_core_.device.unmapMemory2(unmap_info);
        vulkan_core_.vertex_buffer_mapped = nullptr;
      }
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
    if (auto e = init_vertex_buffer(40000); e.is_error())
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
            static_cast<float>(vulkan_core_.swapchainExtent.height),
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

  if (auto e = update_uniform_buffers(); e.is_error()) {
    return e;
  }

  if (auto e = update_vertex_buffer_from_scene(); e.is_error())
    return e;

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

error::Result<bool> ParticleRenderer::update_vertex_buffer_from_scene() {

  const auto &scene_particles = scene_.get_particles();
  std::vector<vulkan_core::Vertex> vertices;
  vertices.reserve(scene_.get_particles_count());

  for (const auto &sp : scene_particles) {
    vulkan_core::Vertex vertex;
    vertex.position = sp.position;
    vertex.mass = sp.mass;
    vertex.visual_id_low = static_cast<uint32_t>(sp.visual_id & 0xFFFFFFFF);
    vertex.visual_id_high =
        static_cast<uint32_t>((sp.visual_id >> 32) & 0xFFFFFFFF);
    vertices.push_back(vertex);
  }
  update_vertex_buffer(vertices);
  return error::Result<bool>::success(true);
}

error::Result<bool> ParticleRenderer::update_uniform_buffers() {
  auto &frame = vulkan_core_.frames[vulkan_core_.frameIndex];

  // Получаем камеру из сцены
  auto &camera = scene_.get_camera();

  // Создаем UBO
  RenderUniform ubo;
  ubo.view = camera.get_view_matrix();

  // Создаем проекционную матрицу
  float aspect = static_cast<float>(vulkan_core_.swapchainExtent.width) /
                 static_cast<float>(vulkan_core_.swapchainExtent.height);
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

error::Result<bool>
ParticleRenderer::recreate_vertex_buffer(size_t new_capacity) {
  try {
    vulkan_core_.particle_count = scene_.get_particles_count();
    vk::DeviceSize bufferSize = sizeof(vulkan_core::Vertex) * new_capacity;

    // Создаем временный staging буфер
    vk::raii::Buffer stagingBuffer{nullptr};
    vk::raii::DeviceMemory stagingBufferMemory{nullptr};

    create_buffer(vulkan_core_, bufferSize,
                  vk::BufferUsageFlagBits::eTransferSrc,
                  vk::MemoryPropertyFlagBits::eHostVisible |
                      vk::MemoryPropertyFlagBits::eHostCoherent,
                  stagingBuffer, stagingBufferMemory);

    // Маппим staging буфер
    vk::MemoryMapInfo map_info{};
    map_info.memory = *stagingBufferMemory;
    map_info.offset = 0;
    map_info.size = bufferSize;
    map_info.flags = vk::MemoryMapFlags{};

    auto map_result = vulkan_core_.device.mapMemory2(map_info);
    if (!map_result) {
      return error::Result<bool>::error(-1, "Failed to map staging buffer");
    }

    void *data = map_result;
    memcpy(data, scene_.get_particles().data(),
           static_cast<size_t>(bufferSize));

    // Анмаппим
    vk::MemoryUnmapInfo unmap_info{};
    unmap_info.memory = *stagingBufferMemory;
    vulkan_core_.device.unmapMemory2(unmap_info);

    // Создаем конечный vertex буфер
    create_buffer(vulkan_core_, bufferSize,
                  vk::BufferUsageFlagBits::eVertexBuffer |
                      vk::BufferUsageFlagBits::eTransferDst,
                  vk::MemoryPropertyFlagBits::eDeviceLocal,
                  vulkan_core_.vertex_buffer,
                  vulkan_core_.vertex_buffer_memory);

    // Копируем через командный буфер
    vk::raii::CommandBuffer commandBuffer =
        begin_single_time_commands(vulkan_core_);

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    commandBuffer.copyBuffer(*stagingBuffer, *vulkan_core_.vertex_buffer,
                             copyRegion);

    end_single_time_commands(vulkan_core_, commandBuffer);

    debug::debug_print("Created vertex buffer with {} vertices",
                       vulkan_core_.particle_count);
    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    return error::Result<bool>::error(
        -1, std::string(std::string("Failed to recreate vertex buffer: ") +
                        e.what())
                .c_str());
  }
}

error::Result<bool>
ParticleRenderer::init_vertex_buffer(size_t initial_capacity = 50000) {
  try {
    vertex_buffer_capacity_ = initial_capacity;
    vk::DeviceSize buffer_size =
        sizeof(vulkan_core::Vertex) * vertex_buffer_capacity_;

    debug::debug_print("Creating vertex buffer with capacity: {} vertices",
                       vertex_buffer_capacity_);

    vulkan_core::create_buffer(
        vulkan_core_, buffer_size, vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent,
        vulkan_core_.vertex_buffer, vulkan_core_.vertex_buffer_memory);

    // Маппим память с использованием новых API
    vk::MemoryMapInfo map_info{};
    map_info.memory = *vulkan_core_.vertex_buffer_memory;
    map_info.offset = 0;
    map_info.size = buffer_size;
    map_info.flags = vk::MemoryMapFlags{};

    auto map_result = vulkan_core_.device.mapMemory2(map_info);
    if (!map_result) {
      return error::Result<bool>::error(-1,
                                        "Failed to map vertex buffer memory");
    }

    vulkan_core_.vertex_buffer_mapped = map_result;
    vulkan_core_.particle_count = 0;

    debug::debug_print("Vertex buffer initialized with {} bytes capacity",
                       buffer_size);
    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    return error::Result<bool>::error(
        -1,
        std::string(std::string("Failed to create vertex buffer: ") + e.what())
            .c_str());
  }
}

error::Result<bool> ParticleRenderer::update_vertex_buffer(
    const std::vector<vulkan_core::Vertex> &vertices) {
  try {
    size_t particle_count = vertices.size();

    // 1. Проверяем, нужно ли пересоздавать буфер
    if (particle_count > vertex_buffer_capacity_) {
      debug::debug_print("Vertex buffer resize needed: {} > {}", particle_count,
                         vertex_buffer_capacity_);

      size_t new_capacity = particle_count * 2;
      if (auto e = recreate_vertex_buffer(new_capacity); e.is_error()) {
        return e;
      }
    }

    // 2. Копируем данные в маппленную память
    if (vulkan_core_.vertex_buffer_mapped && particle_count > 0) {
      size_t data_size = sizeof(vulkan_core::Vertex) * particle_count;
      memcpy(vulkan_core_.vertex_buffer_mapped, vertices.data(), data_size);

      // Если нужно было бы флашить (но у нас HOST_COHERENT):
      // vk::MappedMemoryRange range{};
      // range.memory = *vulkan_core_.vertex_buffer_memory;
      // range.offset = 0;
      // range.size = VK_WHOLE_SIZE;
      // vulkan_core_.device.flushMappedMemoryRanges(range);
    }

    // 3. Обновляем количество частиц для рендеринга
    vulkan_core_.particle_count = particle_count;

    return error::Result<bool>::success(true);

  } catch (const std::exception &e) {
    return error::Result<bool>::error(
        -1,
        std::string(std::string("Failed to update vertex buffer: ") + e.what())
            .c_str());
  }
}

// Или если нужно через staging буфер (альтернативный вариант):
// error::Result<bool> ParticleRenderer::copy_to_vertex_buffer_via_staging(
//     const std::vector<vulkan_core::Vertex> &vertices) {
//   try {
//     size_t particle_count = vertices.size();
//     vk::DeviceSize buffer_size = sizeof(vulkan_core::Vertex) *
//     particle_count;
//
//     // Проверяем capacity
//     if (particle_count > vertex_buffer_capacity_) {
//       if (auto e = recreate_vertex_buffer(particle_count * 2); e.is_error())
//       {
//         return e;
//       }
//     }
//
//     // 1. Создаем staging буфер
//     vk::raii::Buffer staging_buffer{nullptr};
//     vk::raii::DeviceMemory staging_memory{nullptr};
//
//     vulkan_core::create_buffer(vulkan_core_, buffer_size,
//                                vk::BufferUsageFlagBits::eTransferSrc,
//                                vk::MemoryPropertyFlagBits::eHostVisible |
//                                    vk::MemoryPropertyFlagBits::eHostCoherent,
//                                staging_buffer, staging_memory);
//
//     // 2. Копируем данные в staging буфер
//     vk::MemoryMapInfo map_info{};
//     map_info.memory = *staging_memory;
//     map_info.offset = 0;
//     map_info.size = buffer_size;
//     map_info.flags = vk::MemoryMapFlags();
//
//     auto map_result = vulkan_core_.device.mapMemory2(map_info);
//     if (map_result.result != vk::Result::eSuccess) {
//       return error::Result<bool>::error(-1, "Failed to map staging buffer");
//     }
//
//     memcpy(map_result.value, vertices.data(), buffer_size);
//
//     vk::MemoryUnmapInfo unmap_info{};
//     unmap_info.memory = *staging_memory;
//     vulkan_core_.device.unmapMemory2(unmap_info);
//
//     // 3. Копируем через командный буфер
//     vk::raii::CommandBuffer cmd = begin_single_time_commands();
//
//     vk::BufferCopy copy_region{};
//     copy_region.srcOffset = 0;
//     copy_region.dstOffset = 0;
//     copy_region.size = buffer_size;
//
//     cmd.copyBuffer(*staging_buffer, *vulkan_core_.vertex_buffer,
//     copy_region);
//
//     end_single_time_commands(cmd);
//
//     // 4. Обновляем количество частиц
//     vulkan_core_.particle_count = particle_count;
//
//     return error::Result<bool>::success(true);
//
//   } catch (const vk::SystemError &e) {
//     return error::Result<bool>::error(
//         -1, std::string("Failed to copy to vertex buffer via staging: ") +
//                 e.what());
//   }
// }
} // namespace gfx::renderer
