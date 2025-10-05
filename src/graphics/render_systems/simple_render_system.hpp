#ifndef EMP_SIMPLE_RENDER_SYSTEM_HPP
#define EMP_SIMPLE_RENDER_SYSTEM_HPP

#include "graphics/camera.hpp"
#include "graphics/frame_info.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"
#include "vulkan/pipeline.hpp"

//  std
#include <memory>
#include <vector>

namespace emp {
class SimpleRenderSystem {
public:
    SimpleRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, const char *frag_filename,
                       const char *vert_filename, PipelineConfigInfo *config = nullptr);
    ~SimpleRenderSystem();
    SimpleRenderSystem(const SimpleRenderSystem &) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

    template <class IterableObjects>
    void render(
        FrameInfo &frameInfo, IterableObjects &objects,
        std::function<VkDescriptorSet(DescriptorWriter &, int frame_idx, const typename IterableObjects::value_type &)> writeDesc,
        std::function<void(const VkCommandBuffer &, const typename IterableObjects::value_type &)> bindAndDraw);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, size_t push_constant_struct_size = 4);
    void createPipeline(VkRenderPass renderPass, const char *frag_filename, const char *vert_filename,
                        PipelineConfigInfo *config = nullptr);

    Device &device;
    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout {};

    std::unique_ptr<DescriptorSetLayout> render_system_layout;
};
template <class IterableObjects>
void SimpleRenderSystem::render(
    FrameInfo &frameInfo, IterableObjects &objects,
    std::function<VkDescriptorSet(DescriptorWriter &, int frame_idx, const typename IterableObjects::value_type &)> writeDesc,
    std::function<void(const VkCommandBuffer &, const typename IterableObjects::value_type &)> bindAndDraw)
{
    if(pipeline == nullptr) {
        return;
    }

    pipeline->bind(frameInfo.commandBuffer);

    pipeline->bindDescriptorSets(frameInfo.commandBuffer, &frameInfo.globalDescriptorSet, 0U);

    for(auto object : objects) {
        //  writing descriptor set each frame can slow performance
        //  would be more efficient to implement some sort of caching
        VkDescriptorImageInfo image_info;

        DescriptorWriter desc_writer(*render_system_layout, frameInfo.frameDescriptorPool);

        VkDescriptorSet entity_desc_set = writeDesc(desc_writer, frameInfo.frameIndex, object);

        pipeline->bindDescriptorSets(frameInfo.commandBuffer, &entity_desc_set, 1U);

        bindAndDraw(frameInfo.commandBuffer, object);
    }
}
}  //  namespace emp
#endif
