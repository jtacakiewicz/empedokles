#ifndef EMP_COMPUTE_MANAGER_HPP
#define EMP_COMPUTE_MANAGER_HPP
#include "vulkan/device.hpp"
namespace emp {
class ComputeManager {
public:
    ComputeManager(Device &device);
    ~ComputeManager();
    void beginCompute(uint32_t frame_index);
    VkSemaphore endCompute(uint32_t frame_index, VkCommandBuffer &commandBuffer);

private:
    void createSyncObjects();
    void freeSyncObjects();

    std::vector<VkFence> m_compute_in_flight_fences;
    std::vector<VkSemaphore> m_compute_finished_semaphores;
    Device &m_device;
};
};
#endif  //  EMP_COMPUTE_MANAGER_HPP
