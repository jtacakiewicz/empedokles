#ifndef EMP_BUFFER_HPP
#define EMP_BUFFER_HPP
/*
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "device.hpp"

namespace emp {

class Buffer {
public:
    Buffer(Device &device, VkDeviceSize instance_size, uint32_t instance_count, VkBufferUsageFlags usage_flags,
           VkMemoryPropertyFlags memory_property_flags, VkDeviceSize min_offset_alignment = 1);

    ~Buffer();
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    void writeToBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void writeToIndex(void *data, int index);

    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult flushIndex(int index);

    VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo descriptorInfoForIndex(int index);

    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult invalidateIndex(int index);

    [[nodiscard]] VkBuffer getBuffer() const { return m_buffer; }
    [[nodiscard]] void *getMappedMemory() const { return m_mapped; }
    [[nodiscard]] uint32_t getInstanceCount() const { return m_instance_count; }
    [[nodiscard]] VkDeviceSize getInstanceSize() const { return m_instance_size; }
    [[nodiscard]] VkDeviceSize getAlignmentSize() const { return m_instance_size; }
    [[nodiscard]] VkBufferUsageFlags getUsageFlags() const { return m_usage_flags; }
    [[nodiscard]] VkMemoryPropertyFlags getMemoryPropertyFlags() const { return m_memory_property_flags; }
    [[nodiscard]] VkDeviceSize getBufferSize() const { return m_buffer_size; }

private:
    static VkDeviceSize getAlignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

    Device &m_device;
    void *m_mapped = nullptr;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;

    VkDeviceSize m_buffer_size;
    uint32_t m_instance_count;
    VkDeviceSize m_instance_size;
    VkDeviceSize m_alignment_size;
    VkBufferUsageFlags m_usage_flags;
    VkMemoryPropertyFlags m_memory_property_flags;
};

}  //  namespace emp
#endif
