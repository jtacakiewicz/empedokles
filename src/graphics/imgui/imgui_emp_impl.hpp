#ifndef EMP_IMGUI_EMP_IMPL_HPP
#define EMP_IMGUI_EMP_IMPL_HPP
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "scene/scene_defs.hpp"

namespace emp {
struct Device;
struct Renderer;
void ImGuiSetup(ImGui_ImplVulkan_InitInfo init_info, GLFWwindow *window, Device &device, Renderer &rend, VkRenderPass renderPass);
VkDescriptorPool ImGuiGetDescriptorPool(VkDevice device);
void ImGuiRenderBegin();
void ImGuiRenderEnd(VkCommandBuffer commandBuffer);
void ImGuiDestroy();
};
#endif  //  EMP_IMGUI_IMPL_HPP
