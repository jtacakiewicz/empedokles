#include "renderer.hpp"

//  std
#include <array>
#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include "graphics/imgui/imgui_emp_impl.hpp"

namespace emp {

Renderer::Renderer(Window &window, Device &device)
    : m_window { window }
    , m_device { device }
{
    recreateSwapChain();
    createComputeManager();
    createCommandBuffers();
    createComputeCommandBuffers();
#if EMP_USING_IMGUI
    ImGuiSetup(device.getImGuiInitInfo(), window.getGLFWwindow(), device, *this, getSwapChainRenderPass());
#endif
}

Renderer::~Renderer()
{
    freeCommandBuffers();
    freeComputeCommandBuffers();
#if EMP_USING_IMGUI
    ImGuiDestroy();
#endif
}

void Renderer::createComputeManager()
{
    m_compute_manager = std::make_unique<ComputeManager>(m_device);
}
void Renderer::recreateSwapChain()
{
    auto extent = m_window.getExtent();
    while(extent.width == 0 || extent.height == 0) {
        extent = m_window.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(m_device.device());

    if(m_swap_chain == nullptr) {
        m_swap_chain = std::make_unique<SwapChain>(m_device, extent);
    } else {
        std::shared_ptr<SwapChain> oldSwapChain = std::move(m_swap_chain);
        m_swap_chain = std::make_unique<SwapChain>(m_device, extent, oldSwapChain);

        if(!oldSwapChain->compareSwapFormats(*m_swap_chain)) {
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
        }
    }
}

void Renderer::freeComputeCommandBuffers()
{
    vkFreeCommandBuffers(m_device.device(), m_device.getComputeCommandPool(),
                         static_cast<uint32_t>(m_compute_command_buffers.size()), m_compute_command_buffers.data());
    m_compute_command_buffers.clear();
}
void Renderer::createComputeCommandBuffers()
{
    m_compute_command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_device.getComputeCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_compute_command_buffers.size());

    if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_compute_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
void Renderer::createCommandBuffers()
{
    m_graphics_command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_device.getGraphicsCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_graphics_command_buffers.size());

    if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_graphics_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(m_device.device(), m_device.getGraphicsCommandPool(),
                         static_cast<uint32_t>(m_graphics_command_buffers.size()), m_graphics_command_buffers.data());
    m_graphics_command_buffers.clear();
}

VkCommandBuffer Renderer::beginCompute()
{
    assert(!m_isComputeStarted && "Can't begin compute if compute was already started");
    auto commandBuffer = getCurrentComputeCommandBuffer();

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    m_isComputeStarted = true;
    m_compute_manager->beginCompute(getFrameIndex());
    return commandBuffer;
}
void Renderer::endCompute()
{
    assert(m_isComputeStarted && "Can't end compute that has not started");
    auto commandBuffer = getCurrentComputeCommandBuffer();
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record compute command buffer!");
    }
    m_compute_finished_semaphore = m_compute_manager->endCompute(getFrameIndex(), commandBuffer);
    m_isComputeStarted = false;
}
VkCommandBuffer Renderer::beginFrame()
{
    assert(!m_isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = m_swap_chain->acquireNextImage(&m_current_image_index);
    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    m_isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    return commandBuffer;
}

void Renderer::endFrame()
{
    assert(m_isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    auto result = m_swap_chain->submitCommandBuffers(&commandBuffer, &m_current_image_index, &m_compute_finished_semaphore);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized()) {
        m_window.resetWindowResizedFlag();
        recreateSwapChain();
    } else if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_isFrameStarted = false;
    m_current_frame_index = (m_current_frame_index + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swap_chain->getRenderPass();
    renderPassInfo.framebuffer = m_swap_chain->getFrameBuffer(m_current_image_index);

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swap_chain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swap_chain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(m_swap_chain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor {
        { 0, 0 },
        m_swap_chain->getSwapChainExtent()
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
#if EMP_USING_IMGUI
    ImGuiRenderBegin();
#endif
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer command_buffer) const
{
#if EMP_USING_IMGUI
    ImGuiRenderEnd(command_buffer);
#endif
    assert(m_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(command_buffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(command_buffer);
}

}  //  namespace emp
