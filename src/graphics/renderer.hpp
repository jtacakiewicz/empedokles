#ifndef EMP_RENDERER_HPP
#define EMP_RENDERER_HPP

#include <vulkan/vulkan_core.h>
#include "compute/compute_manager.hpp"
#include "io/window.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swap_chain.hpp"

//  std
#include <cassert>
#include <functional>
#include <memory>
#include <vector>

namespace emp {
class Renderer {
public:
    Renderer(Window &window, Device &device);

    ~Renderer();

    Renderer(const Renderer &) = delete;

    Renderer &operator=(const Renderer &) = delete;

    [[nodiscard]] VkRenderPass getSwapChainRenderPass() const { return m_swap_chain->getRenderPass(); }

    [[nodiscard]] float getAspectRatio() const { return m_swap_chain->extentAspectRatio(); }

    [[nodiscard]] bool isFrameInProgress() const { return m_isFrameStarted; }

    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const
    {
        assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
        return m_graphics_command_buffers[m_current_frame_index];
    }
    [[nodiscard]] VkCommandBuffer getCurrentComputeCommandBuffer() const
    {
        assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
        return m_compute_command_buffers[m_current_frame_index];
    }

    [[nodiscard]] int getFrameIndex() const
    {
        assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
        return m_current_frame_index;
    }

    VkCommandBuffer beginCompute();
    void endCompute();

    VkCommandBuffer beginFrame();
    void endFrame();

    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

private:
    void createCommandBuffers();
    void createComputeCommandBuffers();

    void freeCommandBuffers();
    void freeComputeCommandBuffers();

    void recreateSwapChain();
    void createComputeManager();

    Window &m_window;
    Device &m_device;
    std::unique_ptr<SwapChain> m_swap_chain;
    std::unique_ptr<ComputeManager> m_compute_manager;
    std::vector<VkCommandBuffer> m_graphics_command_buffers;
    std::vector<VkCommandBuffer> m_compute_command_buffers;
    VkSemaphore m_compute_finished_semaphore = NULL;

    uint32_t m_current_image_index {};
    uint32_t m_current_frame_index { 0 };
    bool m_isFrameStarted { false };
    bool m_isComputeStarted { false };
    friend ComputeManager;
};
}  //  namespace emp

#endif
