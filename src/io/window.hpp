#ifndef EMP_WINDOW_HPP
#define EMP_WINDOW_HPP

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <string>

namespace emp {

class Window {
public:
    Window(int w, int h, std::string name);

    ~Window();

    Window(const Window &) = delete;

    Window &operator=(const Window &) = delete;

    bool shouldClose() { return glfwWindowShouldClose(window); }

    VkExtent2D getExtent() const { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
    struct Size {
        int width;
        int height;
    };
    Size getSize() const { return { width, height }; }

    bool wasWindowResized() const { return framebufferResized; }

    void resetWindowResizedFlag() { framebufferResized = false; }

    [[nodiscard]] GLFWwindow *getGLFWwindow() const { return window; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    static void windowResizeCallback(GLFWwindow *window, int width, int height);

    void initWindow();

    int frame_buffer_width;
    int frame_buffer_height;
    int width;
    int height;
    bool framebufferResized = false;

    std::string windowName;
    GLFWwindow *window {};
};
}  //  namespace emp

#endif
