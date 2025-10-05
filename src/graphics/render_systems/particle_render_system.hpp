#ifndef EMP_PARTICLE_RENDER_SYSTEM_HPP
#define EMP_PARTICLE_RENDER_SYSTEM_HPP
#include <random>
#include "graphics/frame_info.hpp"
#include "graphics/particle_emit_queue.hpp"
#include "math/math_defs.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"
#include "vulkan/pipeline.hpp"

namespace emp {

class ParticleRenderSystem {
public:
    static constexpr uint32_t MAX_PARTICLE_COUNT = 65536U;
    //  float to 4
    //  vec2 to 8
    //  vec3/vec4 to 16
    struct alignas(16U) ParticleData {
        vec2f position;
        vec2f velocity;
        vec4f color;
        float lifetime;
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    static_assert(sizeof(ParticleData) == 48, "Wrong struct size");

private:
    VkPipelineLayout compute_pipeline_layout {};
    VkPipelineLayout graphics_pipeline_layout {};
    std::unique_ptr<Pipeline> compute_pipeline;
    std::unique_ptr<Pipeline> graphics_pipeline;

    std::vector<std::unique_ptr<Buffer>> SSBO_buffers;
    std::vector<std::unique_ptr<Buffer>> emit_buffers;

    std::unique_ptr<DescriptorSetLayout> SSBO_layout;
    std::unique_ptr<DescriptorSetLayout> emit_buffer_layout;

    std::unique_ptr<DescriptorPool> compute_pool;
    std::vector<VkDescriptorSet> SSBO_descriptors;
    std::vector<VkDescriptorSet> emit_buffer_descriptors;
    Device &m_device;

    void m_initRandomParticles(Device &device, float aspect) { }

    void m_setupStorageBuffers(Device &device);
    void m_setupEmitBuffers(Device &device);
    void m_setupDescriptorsLayout(Device &device);
    void m_createPipeline(Device &device, VkRenderPass render_pass, VkDescriptorSetLayout compute_UBO_layout,
                          VkDescriptorSetLayout render_UBO_layout);

public:
    void compute(const FrameInfo &frame_info, EmitQueue &emit_queue);
    void render(const FrameInfo &frame_info);
    ~ParticleRenderSystem();
    ParticleRenderSystem(Device &device, VkRenderPass render_pass, VkDescriptorSetLayout global_compute_layout,
                         VkDescriptorSetLayout global_render_layout, float aspect);
};
}

#endif  //  !DEBUG
