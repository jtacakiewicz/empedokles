#include "particle_render_system.hpp"
#include "vulkan/swap_chain.hpp"

namespace emp {

std::vector<VkVertexInputBindingDescription> ParticleRenderSystem::ParticleData::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(ParticleData);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}
std::vector<VkVertexInputAttributeDescription> ParticleRenderSystem::ParticleData::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    int location = 0;
    int binding = 0;
    for(auto info : { std::pair(VK_FORMAT_R32G32_SFLOAT, offsetof(ParticleData, position)),
                      std::pair(VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(ParticleData, color)) }) {
        attributeDescriptions.push_back({});
        auto &attr = attributeDescriptions.back();
        attr.binding = binding;
        attr.location = location++;
        attr.offset = info.second;
        attr.format = info.first;
    }

    return attributeDescriptions;
}

void ParticleRenderSystem::m_setupStorageBuffers(Device &device)
{
    SSBO_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        SSBO_buffers[i] = std::make_unique<Buffer>(device, sizeof(ParticleData), MAX_PARTICLE_COUNT,
                                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
}
void ParticleRenderSystem::m_setupEmitBuffers(Device &device)
{
    emit_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        emit_buffers[i] = std::make_unique<Buffer>(device, sizeof(EmitQueue), 1U, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        emit_buffers[i]->map();
    }
}
void ParticleRenderSystem::m_setupDescriptorsLayout(Device &device)
{
    compute_pool = DescriptorPool::Builder(device)
                       .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2U)
                       .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4)
                       .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                       .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                       .build();
    SSBO_layout = DescriptorSetLayout::Builder(device)
                      .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                      .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                      .build();
    SSBO_descriptors.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        DescriptorWriter desc_writer(*SSBO_layout, *compute_pool);

        VkDescriptorBufferInfo storageBufferInfoLastFrame {};
        storageBufferInfoLastFrame.buffer =
            SSBO_buffers[(i - 1 + SwapChain::MAX_FRAMES_IN_FLIGHT) % SwapChain::MAX_FRAMES_IN_FLIGHT]->getBuffer();
        storageBufferInfoLastFrame.offset = 0;
        storageBufferInfoLastFrame.range = sizeof(ParticleData) * MAX_PARTICLE_COUNT;
        desc_writer.writeBuffer(0, &storageBufferInfoLastFrame);

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame {};
        storageBufferInfoCurrentFrame.buffer = SSBO_buffers[i]->getBuffer();
        storageBufferInfoCurrentFrame.offset = 0;
        storageBufferInfoCurrentFrame.range = sizeof(ParticleData) * MAX_PARTICLE_COUNT;
        desc_writer.writeBuffer(1, &storageBufferInfoCurrentFrame);
        desc_writer.build(SSBO_descriptors[i]);
    }
    emit_buffer_layout = DescriptorSetLayout::Builder(device)
                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                             .build();
    emit_buffer_descriptors.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        DescriptorWriter desc_writer(*emit_buffer_layout, *compute_pool);
        VkDescriptorBufferInfo queryBuffer_info {};
        queryBuffer_info.buffer = emit_buffers[i]->getBuffer();
        queryBuffer_info.offset = 0;
        queryBuffer_info.range = sizeof(EmitQueue);
        desc_writer.writeBuffer(0, &queryBuffer_info);
        desc_writer.build(emit_buffer_descriptors[i]);
    }
}
void ParticleRenderSystem::m_createPipeline(Device &device, VkRenderPass render_pass, VkDescriptorSetLayout compute_UBO_layout,
                                            VkDescriptorSetLayout render_UBO_layout)
{
    {
        PipelineConfigInfo config;
        VkDescriptorSetLayout layouts[] = { compute_UBO_layout, SSBO_layout->getDescriptorSetLayout(),
                                            emit_buffer_layout->getDescriptorSetLayout() };
        //  Create pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 3;
        pipelineLayoutInfo.pSetLayouts = layouts;

        vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &compute_pipeline_layout);

        config.pipelineLayout = compute_pipeline_layout;
        compute_pipeline = std::make_unique<Pipeline>(device, "../assets/shaders/particle_update.comp.spv", config);
    }
    {
        PipelineConfigInfo config;
        Pipeline::defaultPipelineConfigInfo(config);
        config.renderPass = render_pass;
        config.attributeDescriptions = ParticleData::getAttributeDescriptions();
        config.bindingDescriptions = ParticleData::getBindingDescriptions();
        config.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &render_UBO_layout;
        vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &graphics_pipeline_layout);

        config.pipelineLayout = graphics_pipeline_layout;
        graphics_pipeline = std::make_unique<Pipeline>(device, "../assets/shaders/particle_draw.vert.spv",
                                                       "../assets/shaders/particle_draw.frag.spv", config);
    }
}
void ParticleRenderSystem::compute(const FrameInfo &frame_info, EmitQueue &emit_queue)
{
    emit_buffers[frame_info.frameIndex]->writeToBuffer(&emit_queue);
    emit_buffers[frame_info.frameIndex]->flush();
    emit_queue.reset();

    auto frame_index = frame_info.frameIndex;
    compute_pipeline->bind(frame_info.commandBuffer);

    const int desc_nb = 3;
    VkDescriptorSet descriptors[desc_nb] = { frame_info.globalDescriptorSet, SSBO_descriptors[frame_index],
                                             emit_buffer_descriptors[frame_index] };
    compute_pipeline->bindDescriptorSets(frame_info.commandBuffer, descriptors, 0, desc_nb);
    vkCmdDispatch(frame_info.commandBuffer, MAX_PARTICLE_COUNT / 256, 1, 1);
}
void ParticleRenderSystem::render(const FrameInfo &frame_info)
{
    auto frame_index = frame_info.frameIndex;
    graphics_pipeline->bind(frame_info.commandBuffer);

    VkDescriptorSet descriptors[] = { frame_info.globalDescriptorSet };
    graphics_pipeline->bindDescriptorSets(frame_info.commandBuffer, descriptors, 0);

    VkDeviceSize offsets[] = { 0 };
    VkBuffer buffers[] = { SSBO_buffers[frame_index]->getBuffer() };

    vkCmdBindVertexBuffers(frame_info.commandBuffer, 0, 1, buffers, offsets);
    vkCmdDraw(frame_info.commandBuffer, MAX_PARTICLE_COUNT, 1, 0, 0);

    //  frame_info.
}
ParticleRenderSystem::~ParticleRenderSystem()
{
    vkDestroyPipelineLayout(m_device.device(), compute_pipeline_layout, NULL);
    vkDestroyPipelineLayout(m_device.device(), graphics_pipeline_layout, NULL);
}
ParticleRenderSystem::ParticleRenderSystem(Device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_compute_layout,
                                           VkDescriptorSetLayout global_render_layout, float aspect)
    : m_device(device)
{
    m_setupStorageBuffers(device);
    m_setupEmitBuffers(device);
    m_initRandomParticles(device, aspect);
    m_setupDescriptorsLayout(device);
    m_createPipeline(device, render_pass, global_compute_layout, global_render_layout);
}
}
