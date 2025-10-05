#ifndef EMP_MODEL_SYSTEM_HPP
#define EMP_MODEL_SYSTEM_HPP

#include "graphics/render_systems/simple_render_system.hpp"
#include "model.hpp"
#include "scene/transform.hpp"
#include "texture.hpp"
#include "vulkan/swap_chain.hpp"

//  libs
#include <glm/gtc/matrix_transform.hpp>

//  std
#include <memory>
#include <unordered_map>

namespace emp {

struct ModelShaderInfo {
    glm::mat4 modelMatrix { 1.f };
    glm::mat4 normalMatrix { 1.f };
    glm::vec4 color { 1.f };
};

class ModelSystem : public System<Transform, Model> {
public:
    ModelSystem(Device &device);

    void render(FrameInfo &frame_info, SimpleRenderSystem &simple_rend_system);

    [[nodiscard]] VkDescriptorBufferInfo getBufferInfoForGameObject(int frameIndex, Entity entity) const
    {
        return uboBuffers[frameIndex]->descriptorInfoForIndex(entity);
    }

    void updateBuffer(int frameIndex);
    std::vector<std::unique_ptr<Buffer>> uboBuffers { SwapChain::MAX_FRAMES_IN_FLIGHT };
};

}  //  namespace emp
#endif
