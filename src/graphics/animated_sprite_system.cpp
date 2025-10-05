#include "animated_sprite_system.hpp"
#include "graphics/sprite_system.hpp"

namespace emp {
void AnimatedSpriteSystem::render(FrameInfo &frame_info, SimpleRenderSystem &simple_rend_system)
{
    simple_rend_system.render(
        frame_info, this->entities,
        [this](DescriptorWriter &desc_writer, int frame_index, const Entity &entity) -> VkDescriptorSet {
            static VkDescriptorBufferInfo buf_info;
            buf_info = getBufferInfoForGameObject(frame_index, entity);
            auto &image_info = getComponent<AnimatedSprite>(entity).sprite().texture().getImageInfo();
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
AnimatedSpriteSystem::AnimatedSpriteSystem(Device &device)
{
    //  including nonCoherentAtomSize allows us to flush a specific index at once
    int alignment =
        std::lcm(device.properties.limits.nonCoherentAtomSize, device.properties.limits.minUniformBufferOffsetAlignment);
    for(auto &uboBuffer : uboBuffers) {
        uboBuffer = std::make_unique<Buffer>(device, sizeof(SpriteInfo), MAX_ENTITIES, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, alignment);
    }
}

void AnimatedSpriteSystem::updateTransitions(float delta_time)
{
    for(auto entity : entities) {
        getComponent<AnimatedSprite>(entity).updateState(entity, delta_time);
    }
}
void AnimatedSpriteSystem::updateBuffer(int frameIndex)
{
    //  copy model matrix and normal matrix for each gameObj into
    //  buffer for this frame
    uboBuffers[frameIndex]->map();
    for(auto entity : entities) {

        //  auto &obj = kv.second;
        const auto &transform = getComponent<Transform>(entity);
        const auto &animated = getComponent<AnimatedSprite>(entity);
        SpriteInfo data {};

        data.model_matrix = transform.global();
        data.offset_matrix =
            glm::translate(glm::mat4x4(1.f), glm::vec3(animated.sprite().position_offset + animated.position_offset, 0));
        data.size_matrix = glm::scale(glm::mat4 { 1.f }, { animated.sprite().size().x, animated.sprite().size().y, 1.f });

        auto pivot = (animated.sprite().centered ? vec2f(0, 0) : animated.sprite().size() * 0.5f);
        data.pivot_matrix = glm::translate(glm::mat4 { 1.f }, glm::vec3(pivot.x, pivot.y, 0.f));

        auto rect = animated.sprite().shader_rect();
        data.rect_min = rect.min;
        data.rect_max = rect.max;

        data.flip = { animated.sprite().flipX ^ animated.flipX, animated.sprite().flipY ^ animated.flipY };  //  only 0.f or 1.f
        data.color = animated.color;
        data.order = animated.sprite().order;
        if(animated.color_override.has_value()) {
            data.color_override = animated.color_override.value();
        }

        uboBuffers[frameIndex]->writeToIndex(&data, entity);
    }
    uboBuffers[frameIndex]->flush();
    uboBuffers[frameIndex]->unmap();
}
};
