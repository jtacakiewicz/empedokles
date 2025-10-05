#include "texture.hpp"
#include <iostream>
#include "vulkan/buffer.hpp"
#include "debug/log.hpp"

//  libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//  std
#include <cmath>
#include <stdexcept>

namespace emp {
Texture::Texture(std::string model_id)
    : m_id(model_id)
{
    if(!isLoaded(model_id)) {
        EMP_LOG(LogLevel::WARNING) << "texture: \"" << model_id << "\" was not loaded, defaulting";
        m_id = "default";
    }
}
AssetRegistry<TextureAsset> Texture::m_assets;
TextureAsset::TextureAsset(Device &device, const std::string &textureFilepath)
    : m_device { device }
{
    createTextureImage(textureFilepath);
    createTextureImageView(VK_IMAGE_VIEW_TYPE_2D);
    createTextureSampler();
    updateDescriptor();
}

TextureAsset::TextureAsset(Device &m_device, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage,
                           VkSampleCountFlagBits sampleCount)
    : m_device { m_device }
{

    VkImageAspectFlags aspectMask = 0;

    m_mip_levels = 1;
    m_format = format;
    m_extent = extent;

    if(m_format & VK_FORMAT_R8G8B8A8_SRGB) {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if(usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if(usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    VkImageCreateInfo image_info {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent = m_extent;
    image_info.mipLevels = m_mip_levels;
    image_info.arrayLayers = m_layer_count;
    image_info.format = m_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = sampleCount;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    m_device.createImageWithInfo(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_texture_image, m_texture_image_memory);

    createTextureImageView(VK_IMAGE_VIEW_TYPE_2D);
    if(usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        createTextureSampler();
    }
    updateDescriptor();
}

TextureAsset::~TextureAsset()
{
    vkDestroySampler(m_device.device(), m_texture_sampler, nullptr);
    vkDestroyImageView(m_device.device(), m_texture_image_view, nullptr);
    vkDestroyImage(m_device.device(), m_texture_image, nullptr);
    vkFreeMemory(m_device.device(), m_texture_image_memory, nullptr);
}

std::unique_ptr<TextureAsset> TextureAsset::createTextureFromFile(Device &device, const std::string &filepath)
{
    return std::make_unique<TextureAsset>(device, filepath);
}

void TextureAsset::updateDescriptor()
{
    m_descriptor.sampler = m_texture_sampler;
    m_descriptor.imageView = m_texture_image_view;
    m_descriptor.imageLayout = m_texture_layout;
}

std::vector<TextureAsset::Pixel> TextureAsset::getPixelsFromGPU()
{

    VkDeviceSize imageSize = getExtent().width * getExtent().height;
    constexpr std::size_t pixel_size = sizeof(stbi_uc) * 4U;

    auto old_layout = m_texture_layout;

    {
        auto command_buffer = m_device.beginSingleTimeCommands();
        transitionLayout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        m_device.endSingleTimeCommands(command_buffer);
    }

    Buffer stagingBuffer {
        m_device,
        pixel_size,
        static_cast<uint32_t>(imageSize),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };
    m_device.copyImageToBuffer(getImage(), stagingBuffer.getBuffer(), getExtent().width, getExtent().height, 1);
    stagingBuffer.map();

    static_assert(std::is_same<unsigned char, stbi_uc>::value);
    static_assert(sizeof(Pixel) == pixel_size);

    std::vector<Pixel> pixels(imageSize);
    memcpy(pixels.data(), stagingBuffer.getMappedMemory(), imageSize * sizeof(Pixel));

    {
        auto command_buffer = m_device.beginSingleTimeCommands();
        transitionLayout(command_buffer, old_layout);
        m_device.endSingleTimeCommands(command_buffer);
    }

    return pixels;
}
void TextureAsset::createTextureImage(const std::string &filepath)
{
    int texWidth, texHeight, texChannels;
    //  stbi_set_flip_vertically_on_load(1);  // todo determine why texture
    //  coordinates are flipped
    stbi_uc *pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if(!pixels) {
        throw std::runtime_error("failed to load image file: " + filepath);
    }

    //  mMipLevels =
    //  static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth,
    //  texHeight)))) + 1;
    m_mip_levels = 1;

    Buffer stagingBuffer {
        m_device,
        sizeof(stbi_uc) * 4U,
        static_cast<uint32_t>(texWidth * texHeight),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    memcpy(stagingBuffer.getMappedMemory(), pixels, static_cast<size_t>(imageSize));
    stagingBuffer.unmap();

    stbi_image_free(pixels);

    m_format = VK_FORMAT_R8G8B8A8_SRGB;
    m_extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };

    VkImageCreateInfo image_info {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent = m_extent;
    image_info.mipLevels = m_mip_levels;
    image_info.arrayLayers = m_layer_count;
    image_info.format = m_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    m_device.createImageWithInfo(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_texture_image, m_texture_image_memory);
    {
        auto command_buffer = m_device.beginSingleTimeCommands();
        transitionLayout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_device.endSingleTimeCommands(command_buffer);
    }
    m_device.copyBufferToImage(stagingBuffer.getBuffer(), m_texture_image, static_cast<uint32_t>(texWidth),
                               static_cast<uint32_t>(texHeight), m_layer_count);

    {
        auto command_buffer = m_device.beginSingleTimeCommands();
        transitionLayout(command_buffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_device.endSingleTimeCommands(command_buffer);
    }

    //  If we generate mip maps then the final image will alerady be
    //  READ_ONLY_OPTIMAL mDevice.generateMipmaps(mTextureImage, mFormat,
    //  texWidth, texHeight, mMipLevels);
    m_texture_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void TextureAsset::createTextureImageView(VkImageViewType viewType)
{
    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_texture_image;
    viewInfo.viewType = viewType;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_mip_levels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = m_layer_count;

    if(vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_texture_image_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void TextureAsset::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    //  these fields useful for percentage close filtering for shadow maps
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(m_mip_levels);

    if(vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_texture_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

VkAccessFlags getAccessFlag(VkImageLayout layout)
{
    switch(layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        case VK_IMAGE_LAYOUT_GENERAL:
            return VK_ACCESS_SHADER_WRITE_BIT;
        default:
            return 0;
    }
}
VkPipelineStageFlags getStage(VkImageLayout layout)
{
    switch(layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case VK_IMAGE_LAYOUT_GENERAL:
            return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        default:
            return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
}
void TextureAsset::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout)
{
    auto oldLayout = m_texture_layout;

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = m_texture_image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = m_mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = m_layer_count;

    if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if(m_format == VK_FORMAT_D32_SFLOAT_S8_UINT || m_format == VK_FORMAT_D24_UNORM_S8_UINT) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    auto sourceStage = getStage(oldLayout);
    auto destinationStage = getStage(newLayout);

    barrier.srcAccessMask = getAccessFlag(oldLayout);
    barrier.dstAccessMask = getAccessFlag(newLayout);
    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    m_texture_layout = newLayout;
}
}  //  namespace emp
