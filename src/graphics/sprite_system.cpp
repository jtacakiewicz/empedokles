#include "sprite_system.hpp"
namespace emp {
SpriteSystem::SpriteSystem(Device &device)
{
    //  including nonCoherentAtomSize allows us to flush a specific index at once
    int alignment =
        std::lcm(device.properties.limits.nonCoherentAtomSize, device.properties.limits.minUniformBufferOffsetAlignment);
    for(auto &uboBuffer : uboBuffers) {
        uboBuffer = std::make_unique<Buffer>(device, sizeof(SpriteInfo), MAX_ENTITIES, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, alignment);
    }
}
void SpriteSystem::render(FrameInfo &frame_info, SimpleRenderSystem &simple_rend_system)
{
    simple_rend_system.render(
        frame_info, this->entities,
        [this](DescriptorWriter &desc_writer, int frame_index, const Entity &entity) -> VkDescriptorSet {
            VkDescriptorBufferInfo buf_info;
            buf_info = getBufferInfoForGameObject(frame_index, entity);
            auto &image_info = getComponent<Sprite>(entity).texture().getImageInfo();
            desc_writer.writeBuffer(0, &buf_info);
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

void SpriteSystem::updateBuffer(int frameIndex)
{
    //  copy model matrix and normal matrix for each gameObj into
    //  buffer for this frame
    uboBuffers[frameIndex]->map();
    for(auto e : entities) {
        //  auto &obj = kv.second;
        const auto &transform = getComponent<Transform>(e);
        const auto &sprite = getComponent<Sprite>(e);
        SpriteInfo data {};

        data.model_matrix = transform.global();
        data.offset_matrix = glm::translate(glm::mat4x4(1.f), glm::vec3(sprite.position_offset, 0));
        data.size_matrix = glm::scale(glm::mat4 { 1.f }, { sprite.size().x, sprite.size().y, 1.f });

        auto pivot = (sprite.centered ? vec2f(0, 0) : sprite.size() * 0.5f);
        data.pivot_matrix = glm::translate(glm::mat4 { 1.f }, glm::vec3(pivot.x, pivot.y, 0.f));

        auto rect = sprite.shader_rect();
        data.rect_min = rect.min;
        data.rect_max = rect.max;

        data.flip = { sprite.flipX, sprite.flipY };  //  only 0.f or 1.f
        if(sprite.color_override.has_value()) {
            data.color_override = sprite.color_override.value();
        }
        data.color = sprite.color;
        data.order = sprite.order;

        uboBuffers[frameIndex]->writeToIndex(&data, e);
    }
    uboBuffers[frameIndex]->flush();
    uboBuffers[frameIndex]->unmap();
}
};  //  namespace emp
