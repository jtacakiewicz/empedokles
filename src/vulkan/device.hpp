#ifndef EMP_DEVICE_HPP
#define EMP_DEVICE_HPP

#include <vulkan/vulkan_core.h>
#include "graphics/imgui/imgui_emp_impl.hpp"
#include "io/window.hpp"

//  std lib headers
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace emp {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily {};
    std::optional<uint32_t> presentFamily {};
    std::optional<uint32_t> computeFamily {};
    std::optional<uint32_t> graphicsNcomputeFamily {};

    bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() &&
               graphicsNcomputeFamily.has_value();
    }
    std::set<uint32_t> getUniqueFamilies() const
    {
        std::set<uint32_t> result;
        for(auto f : { graphicsFamily, presentFamily, computeFamily, graphicsNcomputeFamily }) {
            if(f.has_value()) {
                result.insert(f.value());
            }
        }
        return result;
    }
};

class Device {
public:
#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

    explicit Device(Window &window);

    ~Device();

    //  Not copyable or movable
    Device(const Device &) = delete;

    Device &operator=(const Device &) = delete;

    Device(Device &&) = delete;

    Device &operator=(Device &&) = delete;

    VkCommandPool getGraphicsCommandPool() { return m_graphics_command_pool; }
    VkCommandPool getComputeCommandPool() { return m_compute_command_pool; }
    VkCommandPool getGraphicsComputeCommandPool() { return m_graphics_compute_command_pool; }

    VkDevice device() { return m_device; }

    VkSurfaceKHR surface() { return m_surface; }

    VkQueue graphicsQueue() { return m_graphics_queue; }
    VkQueue computeQueue() { return m_compute_queue; }
    VkQueue graphicsComputeQueue() { return m_graphics_compute_queue; }

    VkQueue presentQueue() { return m_present_queue; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_physical_device); }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physical_device); }

    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    //  Buffer Helper Functions
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void copyImageToImage(VkImage src, VkImage dst, uint32_t width, uint32_t height, uint32_t layerCount);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
    void copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount);

    void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image,
                             VkDeviceMemory &imageMemory);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels = 1, uint32_t layerCount = 1);

    VkPhysicalDeviceProperties properties {};
#if EMP_USING_IMGUI
    ImGui_ImplVulkan_InitInfo getImGuiInitInfo() const;
#endif
    void useSingleTimeCommand(std::function<void(VkCommandBuffer)> &&func);
    void useSingleTimeComputeCommand(std::function<void(VkCommandBuffer)> &&func);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkCommandBuffer beginSingleTimeComputeCommands();
    void endSingleTimeComputeCommands(VkCommandBuffer commandBuffer);

private:
    void createInstance();

    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createCommandPools();

    //  helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);

    std::vector<const char *> getRequiredExtensions() const;

    bool checkValidationLayerSupport();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void hasGflwRequiredInstanceExtensions();

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkInstance m_instance {};
    VkDebugUtilsMessengerEXT m_debug_messenger {};
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    Window &m_window;

    VkCommandPool m_graphics_command_pool {};
    VkCommandPool m_compute_command_pool {};
    VkCommandPool m_graphics_compute_command_pool {};

    VkDevice m_device {};
    VkSurfaceKHR m_surface {};
    VkQueue m_graphics_compute_queue {};
    VkQueue m_compute_queue {};
    VkQueue m_graphics_queue {};
    VkQueue m_present_queue {};

    const std::vector<const char *> validation_layers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char *> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
                                                          VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#endif
    };
};

}  //  namespace emp

#endif
