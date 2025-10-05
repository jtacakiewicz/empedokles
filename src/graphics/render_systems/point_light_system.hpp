#ifndef EMP_POINT_LIGHT_SYSTEM_HPP
#define EMP_POINT_LIGHT_SYSTEM_HPP

#include "graphics/camera.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/game_object.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/vulkan/pipeline.hpp"

//  std
#include <memory>
#include <vector>

namespace emp {
class PointLightSystem {
public:
    PointLightSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

    ~PointLightSystem();

    PointLightSystem(const PointLightSystem &) = delete;

    PointLightSystem &operator=(const PointLightSystem &) = delete;

    static void update(FrameInfo &frameInfo, GlobalUbo &ubo);

    void render(FrameInfo &frameInfo);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);

    void createPipeline(VkRenderPass renderPass);

    Device &device;

    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipelineLayout {};
};
}  //  namespace emp
#endif
