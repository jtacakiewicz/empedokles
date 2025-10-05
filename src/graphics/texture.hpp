#ifndef EMP_TEXTURE_HPP
#define EMP_TEXTURE_HPP

#include "core/asset_registry.hpp"
#include "math/math_defs.hpp"
#include "vulkan/device.hpp"

//  libs
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <unordered_map>

//  std
#include <memory>
#include <string>

namespace emp {
class TextureAsset {
public:
    TextureAsset(Device &device, const std::string &textureFilepath);

    TextureAsset(Device &device, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount);

    ~TextureAsset();

    //  delete copy constructors
    TextureAsset(const TextureAsset &) = delete;

    TextureAsset &operator=(const TextureAsset &) = delete;

    [[nodiscard]] VkImageView imageView() const { return m_texture_image_view; }

    [[nodiscard]] VkSampler sampler() const { return m_texture_sampler; }

    [[nodiscard]] VkImage getImage() const { return m_texture_image; }

    [[nodiscard]] VkImageView getImageView() const { return m_texture_image_view; }

    [[nodiscard]] const VkDescriptorImageInfo &getImageInfo() const { return m_descriptor; }
    [[nodiscard]] VkDescriptorImageInfo &getImageInfo() { return m_descriptor; }

    [[nodiscard]] VkImageLayout getImageLayout() const { return m_texture_layout; }

    [[nodiscard]] VkExtent3D getExtent() const { return m_extent; }
    [[nodiscard]] vec2f getSize() const { return { static_cast<float>(m_extent.width), static_cast<float>(m_extent.height) }; }

    [[nodiscard]] VkFormat getFormat() const { return m_format; }

    void updateDescriptor();

    void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);
    struct Pixel {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
    };
    std::vector<Pixel> getPixelsFromGPU();

    static std::unique_ptr<TextureAsset> createTextureFromFile(Device &device, const std::string &filepath);

private:
    void createTextureImage(const std::string &filepath);
    void createTextureImageView(VkImageViewType viewType);
    void createTextureSampler();

    VkDescriptorImageInfo m_descriptor {};

    Device &m_device;
    VkImage m_texture_image = nullptr;
    VkDeviceMemory m_texture_image_memory = nullptr;
    VkImageView m_texture_image_view = nullptr;
    VkSampler m_texture_sampler = nullptr;
    VkFormat m_format;
    VkImageLayout m_texture_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t m_mip_levels { 1 };
    uint32_t m_layer_count { 1 };
    VkExtent3D m_extent {};
};
class Texture {
private:
    std::string m_id;
    static AssetRegistry<TextureAsset> m_assets;

public:
    static void destroyAll() { m_assets.removeAll(); }
    TextureAsset &texture() { return m_assets.get(m_id); }
    const TextureAsset &texture() const { return m_assets.get(m_id); }
    std::string getID() const { return m_id; }
    static Texture create(std::string id, Device &device, const std::string &filepath)
    {
        m_assets.create(id, std::ref(device), filepath);
        return Texture(id);
    }
    static Texture create(std::string id, Device &device, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage,
                          VkSampleCountFlagBits sampleCount)
    {
        m_assets.create(id, std::ref(device), format, extent, usage, sampleCount);
        return Texture(id);
    }
    static bool isLoaded(std::string id) { return m_assets.isLoaded(id); }
    Texture()
        : m_id("default")
    {
    }
    Texture(std::string model_id);
};

}  //  namespace emp

#endif
