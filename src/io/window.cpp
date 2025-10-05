#include "window.hpp"

//  std
#include <stdexcept>
#include <utility>
#include "debug/log.hpp"

namespace emp {

Window::Window(int w, int h, std::string name)
    : width { w }
    , height { h }
    , windowName { std::move(std::move(name)) }
{
    initWindow();
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    glfwGetFramebufferSize(window, &frame_buffer_width, &frame_buffer_height);
    glfwGetWindowSize(window, &width, &height);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto pwindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    pwindow->framebufferResized = true;
    pwindow->frame_buffer_width = width;
    pwindow->frame_buffer_height = height;
}
void Window::windowResizeCallback(GLFWwindow *window, int width, int height)
{
    auto pwindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    pwindow->framebufferResized = true;
    pwindow->width = width;
    pwindow->height = height;
}

}  //  namespace emp
