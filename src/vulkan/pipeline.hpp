#ifndef EMP_PIPELINE_HPP
#define EMP_PIPELINE_HPP

#include <vulkan/vulkan_core.h>
#include "vulkan/device.hpp"

//  std
#include <string>
#include <vector>

namespace emp {

struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo &) = delete;
    PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions {};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions {};
    VkPipelineViewportStateCreateInfo viewportInfo {};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo {};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo {};
    VkPipelineMultisampleStateCreateInfo multisampleInfo {};
    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo {};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo {};
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo {};
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline {
public:
    Pipeline(Device &device, const std::string &vertFilepath, const std::string &fragFilepath,
             const PipelineConfigInfo &configInfo);
    Pipeline(Device &device, const std::string &computeFilepath, const PipelineConfigInfo &configInfo);

    ~Pipeline();
    Pipeline(const Pipeline &) = delete;
    Pipeline &operator=(const Pipeline &) = delete;
    void bind(VkCommandBuffer commandBuffer);
    void bindDescriptorSets(VkCommandBuffer command_buffer, VkDescriptorSet *sets, uint32_t first_set, uint32_t set_count = 1,
                            uint32_t dynamic_offset_count = 0U, uint32_t *pDynamic_offsets = nullptr);

    static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);
    static void enableAlphaBlending(PipelineConfigInfo &configInfo);

private:
    static std::vector<char> readFile(const std::string &filepath);

    void createComputePipeline(const std::string &computeFilepath, const PipelineConfigInfo &configInfo);
    void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath,
                                const PipelineConfigInfo &configInfo);

    void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

    VkPipelineLayout m_layout;
    Device &m_device;
    VkPipeline m_pipeline {};
    VkShaderModule m_vert_shader_module {};
    VkShaderModule m_frag_shader_module {};
    VkShaderModule m_compute_shader_module {};
};
}  //  namespace emp

#endif
