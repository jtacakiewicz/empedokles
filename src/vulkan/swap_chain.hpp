#ifndef EMP_SWAP_CHAIN_HPP
#define EMP_SWAP_CHAIN_HPP

#include "vulkan/device.hpp"

//  vulkan headers
#include <vulkan/vulkan.h>

//  std lib headers
#include <memory>
#include <string>
#include <vector>

namespace emp {

class SwapChain {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(Device &deviceRef, VkExtent2D windowExtent);

    SwapChain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);

    ~SwapChain();

    SwapChain(const SwapChain &) = delete;

    SwapChain &operator=(const SwapChain &) = delete;

    VkFramebuffer getFrameBuffer(int index) { return m_swapChain_framebuffers[index]; }

    VkRenderPass getRenderPass() { return m_render_pass; }

    VkImageView getImageView(int index) { return m_swapChain_image_views[index]; }

    size_t imageCount() { return m_swapChain_images.size(); }

    VkFormat getSwapChainImageFormat() { return m_swapChain_image_format; }

    VkExtent2D getSwapChainExtent() { return m_swapChain_extent; }

    uint32_t width() const { return m_swapChain_extent.width; }

    uint32_t height() const { return m_swapChain_extent.height; }

    float extentAspectRatio() const
    {
        return static_cast<float>(m_swapChain_extent.width) / static_cast<float>(m_swapChain_extent.height);
    }

    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t *imageIndex);

    VkResult submitCommandBuffers(const VkCommandBuffer *buffers, const uint32_t *imageIndex,
                                  VkSemaphore *compute_finished = NULL);

    [[nodiscard]] bool compareSwapFormats(const SwapChain &swapChain) const
    {
        return swapChain.m_swapChain_depth_format == m_swapChain_depth_format &&
               swapChain.m_swapChain_image_format == m_swapChain_image_format;
    }

private:
    void init();

    void createSwapChain();

    void createImageViews();

    void createDepthResources();

    void createRenderPass();

    void createFramebuffers();

    void createSyncObjects();

    //  Helper functions
    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat m_swapChain_image_format;
    VkFormat m_swapChain_depth_format;
    VkExtent2D m_swapChain_extent {};

    std::vector<VkFramebuffer> m_swapChain_framebuffers;
    VkRenderPass m_render_pass {};

    std::vector<VkImage> m_depth_images;
    std::vector<VkDeviceMemory> m_depth_image_memories;
    std::vector<VkImageView> m_depth_image_views;
    std::vector<VkImage> m_swapChain_images;
    std::vector<VkImageView> m_swapChain_image_views;

    Device &m_device;
    VkExtent2D m_window_extent;

    VkSwapchainKHR m_swapChain {};
    std::shared_ptr<SwapChain> m_old_swapChain;

    std::vector<VkSemaphore> m_image_available_semaphores;
    std::vector<VkSemaphore> m_render_finished_semaphores;
    std::vector<VkFence> m_in_flight_fences;
    std::vector<VkFence> m_images_in_flight;
    size_t m_current_frame = 0;
};

}  //  namespace emp

#endif
