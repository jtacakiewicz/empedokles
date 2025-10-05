#include "compute_manager.hpp"
#include <vulkan/vulkan_core.h>
#include "vulkan/swap_chain.hpp"
namespace emp {
ComputeManager::ComputeManager(Device &device)
    : m_device(device)
{
    createSyncObjects();
}
ComputeManager::~ComputeManager()
{
    freeSyncObjects();
}

void ComputeManager::freeSyncObjects()
{
    for(size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device.device(), m_compute_finished_semaphores[i], nullptr);
        vkDestroyFence(m_device.device(), m_compute_in_flight_fences[i], nullptr);
    }
}
void ComputeManager::createSyncObjects()
{
    m_compute_in_flight_fences.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    m_compute_finished_semaphores.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        if(vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_compute_finished_semaphores[i]) != VK_SUCCESS ||
           vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_compute_in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute synchronization objects for a "
                                     "frame!");
        }
    }
}
void ComputeManager::beginCompute(uint32_t frame_index)
{
    vkWaitForFences(m_device.device(), 1, &m_compute_in_flight_fences[frame_index], VK_TRUE, UINT64_MAX);
    vkResetFences(m_device.device(), 1, &m_compute_in_flight_fences[frame_index]);
}
VkSemaphore ComputeManager::endCompute(uint32_t frame_index, VkCommandBuffer &commandBuffer)
{
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_compute_finished_semaphores[frame_index];
    if(vkQueueSubmit(m_device.computeQueue(), 1, &submitInfo, m_compute_in_flight_fences[frame_index]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    };
    return m_compute_finished_semaphores[frame_index];
}
};
