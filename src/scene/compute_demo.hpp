#pragma once

#include "debug/log.hpp"
#include "graphics/sprite.hpp"
#include <vulkan/vulkan_core.h>
#include <memory>
#include "graphics/frame_info.hpp"
#include "graphics/sprite_system.hpp"
#include "graphics/render_systems/simple_render_system.hpp"
#include "graphics/texture.hpp"
#include "utils/time.hpp"
#include "vulkan/pipeline.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/swap_chain.hpp"
#include "vulkan/descriptors.hpp"
namespace emp {
class ComputeDemo {
public:
    Device &m_device;
    std::unique_ptr<DescriptorSetLayout> layoutBinding;
    std::unique_ptr<DescriptorPool> compute_pool;
    const size_t dataCount = 256 * 256;
    static const uint32_t MAX_FRAMES_IN_FLIGHT = SwapChain::MAX_FRAMES_IN_FLIGHT;
    VkDescriptorSet descriptor_set[MAX_FRAMES_IN_FLIGHT];

    VkPipelineLayout pipeline_layout {};
    std::unique_ptr<Pipeline> compute_pipeline;

    std::unique_ptr<Buffer> dataBuffer[MAX_FRAMES_IN_FLIGHT];
    std::vector<uint32_t> inputData;

    std::unique_ptr<TextureAsset> output_image[MAX_FRAMES_IN_FLIGHT];
    std::unique_ptr<TextureAsset> display_image[MAX_FRAMES_IN_FLIGHT];
    Transform transform;
    vec2f size = { 900.f, 900.f };
    std::vector<std::unique_ptr<Buffer>> uboBuffers { SwapChain::MAX_FRAMES_IN_FLIGHT };

    void copyImageToImage(VkCommandBuffer command_buffer, VkImage src, VkImage dst, uint32_t width, uint32_t height,
                          uint32_t layerCount)
    {
        VkImageCopy region {};
        region.extent = { width, height, 1 };

        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.mipLevel = 0;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.layerCount = layerCount;

        region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstSubresource.mipLevel = 0;
        region.dstSubresource.baseArrayLayer = 0;
        region.dstSubresource.layerCount = layerCount;

        region.dstOffset = { 0, 0, 0 };
        region.srcOffset = { 0, 0, 0 };

        vkCmdCopyImage(command_buffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                       &region);
    }
    void initializeImages(Device &device)
    {
        VkExtent3D extent = { 256, 256, 1 };
        assert(extent.width * extent.height * extent.depth == dataCount);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            output_image[i] = std::make_unique<TextureAsset>(
                device, VK_FORMAT_R8G8B8A8_SRGB, extent,
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_SAMPLE_COUNT_1_BIT);
            display_image[i] = std::make_unique<TextureAsset>(device, VK_FORMAT_R8G8B8A8_SRGB, extent,
                                                              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                              VK_SAMPLE_COUNT_1_BIT);
            {
                auto cmd = device.beginSingleTimeCommands();
                output_image[i]->transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);
                output_image[i]->updateDescriptor();
                device.endSingleTimeCommands(cmd);
            }
        }
    }
    void initializeDataBuffer(Device &device)
    {
        VkDeviceSize bufferSize = sizeof(uint32_t) * dataCount;
        inputData = std::vector<uint32_t>(dataCount, 0);  //  Sample input data: all values set to 2.0
        for(int i = 100; i < 150; i++) {
            for(int ii = 100; ii < 150; ii++) {
                inputData[ii + i * 256] = 1;
            }
        }

        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            //  Create buffer
            dataBuffer[i] = std::make_unique<Buffer>(device, sizeof(uint32_t), dataCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            {
                dataBuffer[i]->map();
                memcpy(dataBuffer[i]->getMappedMemory(), inputData.data(), bufferSize);
                dataBuffer[i]->unmap();
            }
        }
    }
    void createDescriptors(Device &device)
    {
        //  Create descriptor set layout
        layoutBinding = DescriptorSetLayout::Builder(device)
                            .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                            .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                            .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                            .build();

        //  Descriptor pool and set allocation
        compute_pool = DescriptorPool::Builder(device)
                           .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2)
                           .build();
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            auto bufferInfoDSIn = dataBuffer[i]->descriptorInfo();
            auto bufferInfoDSOut = dataBuffer[1 - i]->descriptorInfo();
            DescriptorWriter(*layoutBinding, *compute_pool)
                .writeBuffer(0, &bufferInfoDSIn)
                .writeBuffer(1, &bufferInfoDSOut)
                .writeImage(2, &output_image[1 - i]->getImageInfo())
                .build(descriptor_set[i]);
        }
    }
    void createPipeline(Device &device)
    {
        PipelineConfigInfo config;
        {
            auto layouts = layoutBinding->getDescriptorSetLayout();
            //  Create pipeline layout
            VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &layouts;

            vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout);

            config.pipelineLayout = pipeline_layout;
        }
        compute_pipeline = std::make_unique<Pipeline>(device, "../assets/shaders/falling_sand.comp.spv", config);
    }
    ComputeDemo(Device &device)
        : m_device(device)
        , transform(vec2f(0, 0), 0.f, vec2f(1.f, 1.f))
    {
        transform.syncWithChange();
        initializeImages(m_device);
        initializeDataBuffer(m_device);
        createDescriptors(m_device);
        createPipeline(m_device);
        initializeUBO(m_device);
    }
    void initializeUBO(Device &device)
    {
        int alignment =
            std::lcm(device.properties.limits.nonCoherentAtomSize, device.properties.limits.minUniformBufferOffsetAlignment);
        for(auto &uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(device, sizeof(SpriteInfo), 1U, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, alignment);
        }
    }
    void updateUBO(int frameIndex)
    {
        SpriteInfo data {};

        data.model_matrix = transform.global();
        data.size_matrix = glm::scale(glm::mat4 { 1.f }, { size.x, size.y, 1.f });

        uboBuffers[frameIndex]->map();
        uboBuffers[frameIndex]->writeToBuffer(&data);
        uboBuffers[frameIndex]->unmap();
    }
    void render(FrameInfo &frame_info, SimpleRenderSystem &simple_rend_system)
    {
        updateUBO(frame_info.frameIndex);
        static const std::set<Entity> singleton { 0 };
        simple_rend_system.render(
            frame_info, singleton,
            [this](DescriptorWriter &desc_writer, int frame_index, const Entity &) -> VkDescriptorSet {
                auto buf_info = uboBuffers[frame_index]->descriptorInfo();
                desc_writer.writeBuffer(0, &buf_info);
                auto &image_info = output_image[1 - frame_index]->getImageInfo();
                desc_writer.writeImage(1, &image_info);
                VkDescriptorSet result;
                desc_writer.build(result);
                return result;
            },
            [](const VkCommandBuffer &command_buf, const Entity &entity) {
                Sprite::bind(command_buf);
                Sprite::draw(command_buf);
            });
    }
    void performCompute(FrameInfo frame_info)
    {
        auto command_buffer = frame_info.commandBuffer;
        auto frame_index = frame_info.frameIndex;

        {
            display_image[frame_index]->transitionLayout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            output_image[frame_index]->transitionLayout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        }
        copyImageToImage(command_buffer, output_image[frame_index]->getImage(), display_image[frame_index]->getImage(), 256, 256,
                         1);
        {
            display_image[frame_index]->transitionLayout(command_buffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            output_image[frame_index]->transitionLayout(command_buffer, VK_IMAGE_LAYOUT_GENERAL);
        }
        display_image[frame_index]->updateDescriptor();

        compute_pipeline->bind(command_buffer);
        compute_pipeline->bindDescriptorSets(command_buffer, &descriptor_set[frame_info.frameIndex], 0U);

        auto workgroup_count = 16;
        vkCmdDispatch(command_buffer, workgroup_count, workgroup_count, 1);  //  Dispatch work
    }
};
};
