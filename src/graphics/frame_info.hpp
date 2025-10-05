#ifndef EMP_FRAME_INFO_HPP
#define EMP_FRAME_INFO_HPP

#include "camera.hpp"
#include "vulkan/descriptors.hpp"

//  lib
#include <vulkan/vulkan.h>

namespace emp {

#define MAX_LIGHTS 10

struct PointLight {
    glm::vec4 position {};  //  ignore w
    glm::vec4 color {};     //  w is intensity
};

struct GlobalComputeUbo {
    float delta_time;
};
struct GlobalUbo {
    glm::mat4 projection { 1.f };
    glm::mat4 view { 1.f };
    glm::mat4 inverseView { 1.f };
    glm::vec4 ambientLightColor { 1.f, 1.f, 1.f, .02f };  //  w is intensity
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
};

struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    Camera &camera;
    VkDescriptorSet globalDescriptorSet;
    DescriptorPool &frameDescriptorPool;  //  pool of descriptors that is cleared
                                          //  each frame
};
}  //  namespace emp

#endif
