#include "gfx/vulkan_core/pipeline.h"
#include "ctx/config.h"
#include "utils/namespaces/error_namespace.h"
#include <fstream>

namespace vulkan_core {
static std::vector<char> load_shader(const std::string &filename) {
  std::string dir = std::string(SHADER_DIRECTORY);
  std::string filepath = dir.empty() ? filename : dir + "/" + filename;

  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    std::string err_msg = filename + ": cound not load";
    throw std::runtime_error(err_msg);
  }

  size_t file_size = (size_t)file.tellg();
  debug::debug_print("{} size is {}", filename, file_size);
  std::vector<char> buffer(file_size);

  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();

  return buffer;
}

error::Result<bool> create_graphics_pipeline(VulkanCore &core) {
  debug::debug_print("Starting pipeline creation...");
  if (!*core.render_pass) {
    auto result = create_render_pass(core);
    if (result.is_error())
      return result;
  }

  debug::debug_print("Render pass created");
  try {
    auto vertShader = load_shader("vertex.spv");
    auto fragShader = load_shader("fragment.spv");

    vk::ShaderModuleCreateInfo vertShaderInfo{};
    vertShaderInfo.codeSize = vertShader.size();
    vertShaderInfo.pCode =
        reinterpret_cast<const uint32_t *>(vertShader.data());

    vk::ShaderModuleCreateInfo fragShaderInfo{};
    fragShaderInfo.codeSize = fragShader.size();
    fragShaderInfo.pCode =
        reinterpret_cast<const uint32_t *>(fragShader.data());

    vk::raii::ShaderModule vertShaderModule(core.device, vertShaderInfo);
    vk::raii::ShaderModule fragShaderModule(core.device, fragShaderInfo);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.setPNext(nullptr);
    vertShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = *vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.setPNext(nullptr);
    fragShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = *fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    vk::PipelineShaderStageCreateInfo shader_stages[] = {vertShaderStageInfo,
                                                         fragShaderStageInfo};
    std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport,
                                                    vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.dynamicStateCount =
        static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    vk::VertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = vk::VertexInputRate::eVertex;

    vk::VertexInputAttributeDescription attr_pos{};
    attr_pos.binding = 0;
    attr_pos.location = 0;
    attr_pos.format = vk::Format::eR32G32B32Sfloat;
    attr_pos.offset = offsetof(Vertex, position);

    vk::VertexInputAttributeDescription attr_mass{};
    attr_mass.binding = 0;
    attr_mass.location = 1;
    attr_mass.format = vk::Format::eR32Sfloat;
    attr_mass.offset = offsetof(Vertex, mass);

    vk::VertexInputAttributeDescription attr_visual_low{};
    attr_visual_low.binding = 0;
    attr_visual_low.location = 2; // location = 2 для первой части
    attr_visual_low.format = vk::Format::eR32Uint;
    attr_visual_low.offset = offsetof(Vertex, visual_id_low);

    vk::VertexInputAttributeDescription attr_visual_high{};
    attr_visual_high.binding = 0;
    attr_visual_high.location = 3; // location = 3 для второй части
    attr_visual_high.format = vk::Format::eR32Uint;
    attr_visual_high.offset = offsetof(Vertex, visual_id_high);

    std::array<vk::VertexInputAttributeDescription, 4> attribute_descriptions =
        {attr_pos, attr_mass, attr_visual_low, attr_visual_high};

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions =
        attribute_descriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.topology = vk::PrimitiveTopology::ePointList;
    // input_assembly.topology = vk::PrimitiveTopology::eTriangleList;
    input_assembly.primitiveRestartEnable = VK_FALSE;

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

    vk::PipelineViewportStateCreateInfo viewport_state{};
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.setRasterizerDiscardEnable(VK_FALSE);
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    // rasterizer.polygonMode = vk::PolygonMode::ePoint;
    rasterizer.lineWidth = 1.0f;
    // rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.cullMode = vk::CullModeFlagBits::eNone;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.alphaToCoverageEnable = VK_FALSE;

    vk::PipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    color_blend_attachment.dstColorBlendFactor =
        vk::BlendFactor::eOneMinusSrcAlpha;
    color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
    color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;
    color_blend_attachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendStateCreateInfo color_blending{};
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = vk::LogicOp::eCopy;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    vk::DescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutCreateInfo layout_info{};
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_layout_binding;

    core.descriptor_set_layout =
        vk::raii::DescriptorSetLayout(core.device, layout_info);

    vk::PipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &*core.descriptor_set_layout;

    core.pipeline_layout =
        vk::raii::PipelineLayout(core.device, pipeline_layout_info);

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shader_stages;
    pipelineInfo.pVertexInputState = &vertex_input_info;
    pipelineInfo.pInputAssemblyState = &input_assembly;
    pipelineInfo.pViewportState = &viewport_state;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &color_blending;
    pipelineInfo.pDynamicState = &dynamic_state;
    pipelineInfo.layout = *core.pipeline_layout;
    pipelineInfo.renderPass = *core.render_pass;
    pipelineInfo.subpass = 0;

    core.gfx_pipeline = vk::raii::Pipeline(core.device, nullptr, pipelineInfo);

    debug::debug_print("Graphics pipeline created successfully");
    return error::Result<bool>::success(true);
  } catch (vk::SystemError &error) {
    debug::debug_print("Error in create_graphics_pipeline: %s", error.what());
    return error::Result<bool>::error(-1, error.what());
  }
}

error::Result<bool> create_render_pass(VulkanCore &core) {
  try {
    vk::AttachmentDescription color_attachment{};
    color_attachment.format = core.swapchainImageFormat;
    color_attachment.samples = vk::SampleCountFlagBits::e1;
    color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    color_attachment.initialLayout = vk::ImageLayout::eUndefined;
    color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    vk::RenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;

    core.render_pass =
        vk::raii::RenderPass(core.device, render_pass_create_info);
    debug::debug_print("RenderPass created successfully");

    return error::Result<bool>::success(true);
  } catch (vk::SystemError &error) {
    debug::debug_print("Failed to create render pass: {}", error.what());
    return error::Result<bool>::error(-1, error.what());
  }
}

} // namespace vulkan_core
