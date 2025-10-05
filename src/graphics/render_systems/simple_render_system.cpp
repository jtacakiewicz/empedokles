#include "simple_render_system.hpp"
#include "core/coordinator.hpp"

//  libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//  std
#include <array>
#include <cassert>
#include <stdexcept>

namespace emp {

SimpleRenderSystem::SimpleRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout,
                                       const char *vert_filename, const char *frag_filename, PipelineConfigInfo *config)
    : device { device }
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass, vert_filename, frag_filename, config);
}

SimpleRenderSystem::~SimpleRenderSystem()
{
    vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
}

void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, size_t push_const_size)
{

    render_system_layout =
        DescriptorSetLayout::Builder(device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts { globalSetLayout, render_system_layout->getDescriptorSetLayout() };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass, const char *vert_filename, const char *frag_filename,
                                        PipelineConfigInfo *config)
{
    assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
    PipelineConfigInfo pipelineConfig {};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    if(config == nullptr) {
        config = &pipelineConfig;
    }

    config->renderPass = renderPass;
    config->pipelineLayout = pipeline_layout;
    pipeline = std::make_unique<Pipeline>(device, vert_filename, frag_filename, *config);
}

}  //  namespace emp
