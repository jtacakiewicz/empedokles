#ifndef EMP_SPRITE_SYSTEM_HPP
#define EMP_SPRITE_SYSTEM_HPP
#include "graphics/sprite.hpp"
#include "graphics/render_systems/simple_render_system.hpp"
#include "scene/transform.hpp"
#include "vulkan/swap_chain.hpp"
namespace emp {
struct SpriteInfo {
    glm::mat4 model_matrix { 1.f };
    glm::mat4 offset_matrix { 1.f };
    glm::mat4 pivot_matrix { 1.f };
    glm::mat4 size_matrix { 1.f };

    glm::vec4 color = { 1, 1, 1, 1 };
    glm::vec4 color_override = { 1, 0, 1, 0 };

    glm::vec2 rect_min = { 0, 0 };
    glm::vec2 rect_max = { 1, 1 };
    glm::vec2 flip = { 0, 0 };  //  only 0.f or 1.f
    float order;
    float padding;
};
struct SpriteSystem : public System<Sprite, Transform> {
    SpriteSystem(Device &device);

    [[nodiscard]] VkDescriptorBufferInfo getBufferInfoForGameObject(int frameIndex, Entity entity) const
    {
        return uboBuffers[frameIndex]->descriptorInfoForIndex(entity);
    }

    void render(FrameInfo &frame_info, SimpleRenderSystem &simple_rend_system);

    void updateBuffer(int frameIndex);
    std::vector<std::unique_ptr<Buffer>> uboBuffers { SwapChain::MAX_FRAMES_IN_FLIGHT };
};
};  //  namespace emp
#endif
