#include "gfx/renderer/uniform_data.h"
#include "gfx/vulkan_core/types.h"
#include "utils/namespaces/error_namespace.h"
#include "vulkan/vulkan.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

#include "gfx/vulkan_core/buffer.h"
#include "gfx/vulkan_core/commands.h"
#include "vulkan/vulkan.hpp"

namespace vulkan_core {

void init_frames(VulkanCore &core) {
  assert(core.swapchainImages.size() >= IN_FLIGHT_FRAME_COUNT);

  for (auto &frame : core.frames) {
    vk::CommandBufferAllocateInfo allocInfo{
        *core.commandPool, vk::CommandBufferLevel::ePrimary, 1};
    frame.commandBuffer =
        (std::move(core.device.allocateCommandBuffers(allocInfo).front()));

    // Syncs
    vk::SemaphoreCreateInfo semInfo{};
    frame.imageAvailableSemaphore = core.device.createSemaphore(semInfo);
    frame.renderFinishedSemaphore = core.device.createSemaphore(semInfo);

    vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
    frame.fence = core.device.createFence(fenceInfo);
  }
  debug::debug_print("{} frames inited", core.frames.size());
}

error::Result<bool> record_frame_command_buffer(VulkanCore &core, Frame &frame,
                                                uint32_t imageIndex) {
  try {
    std::cout << "Recording command buffer for image: " << imageIndex << "\n";
    std::cout << "swapchainExtent: " << core.swapchainExtent.width << "x"
              << core.swapchainExtent.height << "\n";

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    frame.commandBuffer.begin(beginInfo);

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(core.swapchainExtent.width);
    viewport.height = static_cast<float>(core.swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = core.swapchainExtent;

    std::array<vk::Viewport, 1> viewports = {viewport};
    std::array<vk::Rect2D, 1> scissors = {scissor};

    frame.commandBuffer.setViewport(0, viewports);
    frame.commandBuffer.setScissor(0, scissors);

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = *core.render_pass;
    renderPassInfo.framebuffer = *core.swapchain_frame_buffers[imageIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = core.swapchainExtent;

    vk::ClearValue clearColor =
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    frame.commandBuffer.beginRenderPass(renderPassInfo,
                                        vk::SubpassContents::eInline);

    // Привязываем пайплайн
    frame.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                     *core.gfx_pipeline);

    // Привязываем вершинный буфер - используем std::array для правильного
    // ArrayProxy
    std::array<vk::Buffer, 1> vertexBuffers = {*core.vertex_buffer};
    std::array<vk::DeviceSize, 1> offsets = {0};
    frame.commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    debug::debug_print("Drawing {} vertices", core.particle_count);

    // Привязываем descriptor set ИМЕННО ЭТОГО КАДРА (текущего frame)
    if (*frame.descriptor_set != nullptr) {
      std::array<vk::DescriptorSet, 1> descriptorSets = {*frame.descriptor_set};
      frame.commandBuffer.bindDescriptorSets(
          vk::PipelineBindPoint::eGraphics, *core.pipeline_layout, 0,
          descriptorSets, {} // пустой массив dynamicOffsets
      );
    } else {
      debug::debug_print("Warning: descriptor set is null for frame");
    }

    // Рисуем
    frame.commandBuffer.draw(static_cast<uint32_t>(core.particle_count), 1, 0,
                             0);

    frame.commandBuffer.endRenderPass();
    frame.commandBuffer.end();

    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    std::cout << "Failed to record frame command buffer" << e.what();
    return error::Result<bool>::error(-1,
                                      "Failed to record frame command buffer");
  }
}

error::Result<bool> create_uniform_buffers(VulkanCore &core) {
  try {
    // vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
    vk::DeviceSize bufferSize = sizeof(gfx::renderer::RenderUniform);

    for (auto &frame : core.frames) {
      create_buffer(core, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible |
                        vk::MemoryPropertyFlagBits::eHostCoherent,
                    frame.uniform_buffer, frame.uniform_buffer_memory);

      vk::MemoryMapInfo mapInfo{};
      mapInfo.memory = *frame.uniform_buffer_memory;
      mapInfo.offset = 0;
      mapInfo.size = bufferSize;
      mapInfo.flags = vk::MemoryMapFlags();

      frame.uniform_buffer_mapped = core.device.mapMemory2(mapInfo);

      debug::debug_print("Created uniform buffer (size: {})", bufferSize);
    }

    return error::Result<bool>::success(true);
  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create uniform buffers: {}", e.what());
    return error::Result<bool>::error(-1, "Failed to create uniform buffers");
  }
}

error::Result<bool> create_descriptor_pool_and_sets(VulkanCore &core) {
  try {
    // 1. Создать descriptor pool
    vk::DescriptorPoolSize poolSize{vk::DescriptorType::eUniformBuffer,
                                    IN_FLIGHT_FRAME_COUNT};
    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &poolSize;
    pool_info.maxSets = IN_FLIGHT_FRAME_COUNT;
    core.descriptor_pool = std::make_unique<vk::raii::DescriptorPool>(
        core.device.createDescriptorPool(pool_info));

    debug::debug_print("Descriptor pool created");

    // 2. Выделить descriptor sets для каждого кадра
    std::vector<vk::DescriptorSetLayout> layouts(IN_FLIGHT_FRAME_COUNT,
                                                 *core.descriptor_set_layout);
    vk::DescriptorSetAllocateInfo alloc_info{};
    alloc_info.descriptorPool = *core.descriptor_pool;
    alloc_info.descriptorSetCount = IN_FLIGHT_FRAME_COUNT;
    alloc_info.pSetLayouts = layouts.data();

    auto sets = core.device.allocateDescriptorSets(alloc_info);

    debug::debug_print("Allocated {} descriptor sets", sets.size());

    // 3. Привязать uniform буферы к descriptor sets
    for (size_t i = 0; i < core.frames.size(); i++) {
      vk::DescriptorBufferInfo buffer_info{};
      buffer_info.buffer = *core.frames[i].uniform_buffer;
      buffer_info.offset = 0;
      buffer_info.range = sizeof(gfx::renderer::RenderUniform);

      vk::WriteDescriptorSet descriptor_write{};
      descriptor_write.dstSet = *sets[i];
      descriptor_write.dstBinding = 0;
      descriptor_write.dstArrayElement = 0;
      descriptor_write.descriptorType = vk::DescriptorType::eUniformBuffer;
      descriptor_write.descriptorCount = 1;
      descriptor_write.pBufferInfo = &buffer_info;

      core.device.updateDescriptorSets(descriptor_write, nullptr);

      core.frames[i].descriptor_set = std::move(sets[i]);

      debug::debug_print("Bound uniform buffer to descriptor set {}", i);
    }

    return error::Result<bool>::success(true);
  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create descriptor pool and sets: {}",
                       e.what());
    return error::Result<bool>::error(
        -1, "Failed to create descriptor pool and sets");
  }
}

error::Result<bool> create_vertex_buffer(VulkanCore &core) {
  std::cout << "Vertex struct size: " << sizeof(Vertex) << ", "
            << "offset of pos: " << offsetof(Vertex, position)
            << "offset of size: " << offsetof(Vertex, mass) << "\n";
  try {
    std::vector<Vertex> vertices{};

    // Пример заполнения тестовыми данными
    // В реальном коде здесь будет заполнение из ваших частиц
    for (size_t i = 0; i < 1000; ++i) {
      Vertex vertex{};
      vertex.position = glm::vec3((rand() / (float)RAND_MAX - 0.5f) * 100.0f,
                                  (rand() / (float)RAND_MAX - 0.5f) * 100.0f,
                                  (rand() / (float)RAND_MAX - 0.5f) * 100.0f);
      vertex.mass = 1.0f + (rand() / (float)RAND_MAX) * 100.0f;

      // Создаем тестовый VisualID
      // В реальном коде это будет браться из ваших частиц
      uint8_t category = rand() % 7; // 0-6
      uint8_t subtype = rand() % 6;  // 0-5
      uint8_t shader = 0;            // базовый шейдер
      uint8_t texture = 0;           // без текстуры
      uint8_t lod = 0;               // базовый LOD
      uint16_t sim_mode = 0;         // стандартный режим
      uint16_t flags = 0;            // без флагов

      VisualID vid = make_visual_id(category, subtype, shader, texture, lod,
                                    sim_mode, flags);

      // Разбиваем 64-битный ID на две 32-битные части
      vertex.visual_id_low = static_cast<uint32_t>(vid.bits & 0xFFFFFFFF);
      vertex.visual_id_high =
          static_cast<uint32_t>((vid.bits >> 32) & 0xFFFFFFFF);

      vertices.push_back(vertex);
    }
    core.particle_count = vertices.size();
    vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    // Создаем временный staging буфер
    vk::raii::Buffer stagingBuffer{nullptr};
    vk::raii::DeviceMemory stagingBufferMemory{nullptr};

    create_buffer(core, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                  vk::MemoryPropertyFlagBits::eHostVisible |
                      vk::MemoryPropertyFlagBits::eHostCoherent,
                  stagingBuffer, stagingBufferMemory);

    vk::MemoryMapInfo mapInfo{};
    mapInfo.memory = *stagingBufferMemory;
    mapInfo.offset = 0;
    mapInfo.size = bufferSize;
    mapInfo.flags = vk::MemoryMapFlags();

    void *data = core.device.mapMemory2(mapInfo);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));

    // WARNING: same here
    vk::MemoryUnmapInfo unmapInfo{};
    unmapInfo.memory = *stagingBufferMemory;
    core.device.unmapMemory2(unmapInfo);

    // Создаем конечный vertex буфер
    create_buffer(core, bufferSize,
                  vk::BufferUsageFlagBits::eVertexBuffer |
                      vk::BufferUsageFlagBits::eTransferDst,
                  vk::MemoryPropertyFlagBits::eDeviceLocal, core.vertex_buffer,
                  core.vertex_buffer_memory);

    // Копируем из временного в конечный буфер
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = *core.commandPool;
    allocInfo.commandBufferCount = 1;

    auto commandBuffers = core.device.allocateCommandBuffers(allocInfo);
    vk::raii::CommandBuffer commandBuffer = std::move(commandBuffers[0]);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    commandBuffer.copyBuffer(*stagingBuffer, *core.vertex_buffer, copyRegion);

    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffer;

    core.graphicsQueue.submit(submitInfo, nullptr);
    core.graphicsQueue.waitIdle();

    debug::debug_print("Created vertex buffer with {} vertices",
                       core.particle_count);
    return error::Result<bool>::success(true);

  } catch (const vk::SystemError &e) {
    debug::debug_print("Failed to create vertex buffer: {}", e.what());
    return error::Result<bool>::error(-1, "Failed to create vertex buffer");
  }
}

} // namespace vulkan_core
