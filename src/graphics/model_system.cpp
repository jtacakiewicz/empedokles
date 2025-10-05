#include "model_system.hpp"
#include "core/coordinator.hpp"

#include <numeric>

namespace emp {
void ModelSystem::render(FrameInfo &frame_info, SimpleRenderSystem &simple_rend_system)
{
    simple_rend_system.render(
        frame_info, this->entities,
        [this](DescriptorWriter &desc_writer, int frame_index, const Entity &entity) -> VkDescriptorSet {
            auto &model = getComponent<Model>(entity);
            VkDescriptorImageInfo &image_info = model.texture.texture().getImageInfo();

            VkDescriptorBufferInfo buf_info;
            buf_info = getBufferInfoForGameObject(frame_index, entity);

            desc_writer.writeBuffer(0, &buf_info);
            desc_writer.writeImage(1, &image_info);
            VkDescriptorSet result;
            desc_writer.build(result);
            return result;
        },
        [this](const VkCommandBuffer &command_buf, const Entity &entity) {
            const auto &model = getComponent<Model>(entity).model();
            model.bind(command_buf);
            model.draw(command_buf);
        });
}

ModelSystem::ModelSystem(Device &device)
{
    //  including nonCoherentAtomSize allows us to flush a specific index at once
    int alignment =
        std::lcm(device.properties.limits.nonCoherentAtomSize, device.properties.limits.minUniformBufferOffsetAlignment);
    for(auto &uboBuffer : uboBuffers) {
        uboBuffer = std::make_unique<Buffer>(device, sizeof(ModelShaderInfo), MAX_ENTITIES, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, alignment);
        uboBuffer->map();
    }

    Texture::create("default", device, "../assets/textures/invalid.png");
}

void ModelSystem::updateBuffer(int frameIndex)
{
    //  copy model matrix and normal matrix for each gameObj into
    //  buffer for this frame
    for(auto e : entities) {
        //  auto &obj = kv.second;
        const auto &transform = getComponent<Transform>(e);
        const auto &model = getComponent<Model>(e);
        ModelShaderInfo data {};
        data.modelMatrix = transform.global();
        data.color = model.color.value_or(glm::vec4 { 1, 1, 1, 1 });
        uboBuffers[frameIndex]->writeToIndex(&data, e);
    }
    uboBuffers[frameIndex]->flush();
}
}  //  namespace emp
