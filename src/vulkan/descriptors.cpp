#include "descriptors.hpp"

//  std
#include <cassert>
#include <stdexcept>

namespace emp {

//  *************** Descriptor Set Layout Builder *********************

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType,
                                                                       VkShaderStageFlags stageFlags, uint32_t count)
{
    assert(bindings.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
{
    return std::make_unique<DescriptorSetLayout>(device, bindings);
}

//  *************** Descriptor Set Layout *********************

DescriptorSetLayout::DescriptorSetLayout(Device &device,
                                         const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> &bindings)
    : m_device { device }
    , m_bindings { bindings }
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings {};
    setLayoutBindings.reserve(bindings.size());
    for(auto kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if(vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutInfo, nullptr, &m_descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_device.device(), m_descriptor_set_layout, nullptr);
}

//  *************** Descriptor Pool Builder *********************

DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
    pool_sizes.push_back({ descriptorType, count });
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
{
    pool_flags = flags;
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count)
{
    max_sets = count;
    return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
{
    return std::make_unique<DescriptorPool>(device, max_sets, pool_flags, pool_sizes);
}

//  *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(Device &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                               const std::vector<VkDescriptorPoolSize> &poolSizes)
    : m_device { device }
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if(vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &m_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(m_device.device(), m_descriptor_pool, nullptr);
}

bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const
{
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptor_pool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    //  Might want to create a "DescriptorPoolManager" class that handles this
    //  case, and builds a new pool whenever an old pool fills up. But this is
    //  beyond our current scope
    if(vkAllocateDescriptorSets(m_device.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const
{
    vkFreeDescriptorSets(m_device.device(), m_descriptor_pool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void DescriptorPool::resetPool()
{
    vkResetDescriptorPool(m_device.device(), m_descriptor_pool, 0);
}

//  *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool)
    : m_set_layout { setLayout }
    , m_pool { pool }
{
}

bool DescriptorWriter::m_isBindingFree(uint32_t binding) const
{
    for(const auto &write : m_writes) {
        if(binding == write.dstBinding) {
            return false;
        }
    }
    return true;
}
DescriptorWriter &DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
{
    assert(m_set_layout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");
    assert(m_isBindingFree(binding) && "cannot overwrite layout binding");

    auto &bindingDescription = m_set_layout.m_bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    m_writes.push_back(write);
    return *this;
}

DescriptorWriter &DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo)
{
    assert(m_set_layout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = m_set_layout.m_bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    m_writes.push_back(write);
    return *this;
}

bool DescriptorWriter::build(VkDescriptorSet &set)
{
    bool success = m_pool.allocateDescriptor(m_set_layout.getDescriptorSetLayout(), set);
    if(!success) {
        return false;
    }
    overwrite(set);
    return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet &set)
{
    for(auto &write : m_writes) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(m_pool.m_device.device(), m_writes.size(), m_writes.data(), 0, nullptr);
}

}  //  namespace emp
