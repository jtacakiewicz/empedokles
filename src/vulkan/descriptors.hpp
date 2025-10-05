#ifndef EMP_DESCRIPTORS_HPP
#define EMP_DESCRIPTORS_HPP

#include "device.hpp"

//  std
#include <memory>
#include <unordered_map>
#include <vector>

namespace emp {

class DescriptorSetLayout {
public:
    class Builder {
    public:
        explicit Builder(Device &device)
            : device { device }
        {
        }

        Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);

        [[nodiscard]] std::unique_ptr<DescriptorSetLayout> build() const;

    private:
        Device &device;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings {};
    };

    DescriptorSetLayout(Device &device, const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> &bindings);

    ~DescriptorSetLayout();

    DescriptorSetLayout(const DescriptorSetLayout &) = delete;

    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

    [[nodiscard]] VkDescriptorSetLayout getDescriptorSetLayout() const { return m_descriptor_set_layout; }

private:
    Device &m_device;
    VkDescriptorSetLayout m_descriptor_set_layout {};
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

    friend class DescriptorWriter;
};

class DescriptorPool {
public:
    class Builder {
    public:
        explicit Builder(Device &device)
            : device { device }
        {
        }

        Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);

        Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);

        Builder &setMaxSets(uint32_t count);

        [[nodiscard]] std::unique_ptr<DescriptorPool> build() const;

    private:
        Device &device;
        std::vector<VkDescriptorPoolSize> pool_sizes {};
        uint32_t max_sets = 1000;
        VkDescriptorPoolCreateFlags pool_flags = 0;
    };

    DescriptorPool(Device &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                   const std::vector<VkDescriptorPoolSize> &poolSizes);

    ~DescriptorPool();

    DescriptorPool(const DescriptorPool &) = delete;

    DescriptorPool &operator=(const DescriptorPool &) = delete;

    bool allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

    void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

    void resetPool();

private:
    Device &m_device;
    VkDescriptorPool m_descriptor_pool {};

    friend class DescriptorWriter;
};

class DescriptorWriter {
public:
    DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

    DescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);

    DescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

    bool build(VkDescriptorSet &set);

    void overwrite(VkDescriptorSet &set);

    DescriptorWriter(const DescriptorWriter &) = delete;
    DescriptorWriter &operator=(const DescriptorWriter &) = delete;

private:
    bool m_isBindingFree(uint32_t binding) const;
    DescriptorSetLayout &m_set_layout;
    DescriptorPool &m_pool;
    std::vector<VkWriteDescriptorSet> m_writes;
};

}  //  namespace emp

#endif
