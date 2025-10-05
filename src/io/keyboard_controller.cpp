#include "keyboard_controller.hpp"
#include <GLFW/glfw3.h>
#include "core/coordinator.hpp"

//  std
#include <limits>
#include "debug/log.hpp"
#include "math/math_func.hpp"
#include "scene/scene_defs.hpp"

namespace emp {

std::unordered_map<int, KeyState> KeyboardController::keys;
vec2f KeyboardController::scroll_velocity = { 0.f, 0.f };

void ScrollCallback(GLFWwindow *window, double xoff, double yoff)
{
    KeyboardController::scroll_velocity = { xoff, yoff };
}
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
#if EMP_USING_IMGUI
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if(ImGui::GetIO().WantCaptureKeyboard) {
        KeyboardController::keys[key].held = false;
        return;
    }
#endif
    if(action == GLFW_PRESS) {
        KeyboardController::keys[key].pressed = true;
        KeyboardController::keys[key].held = true;
    } else if(action == GLFW_RELEASE) {
        KeyboardController::keys[key].released = true;
        KeyboardController::keys[key].held = false;
    }
    KeyboardController::keys[key].mod_flags = mods;
}
void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
#if EMP_USING_IMGUI
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if(ImGui::GetIO().WantCaptureMouse) {
        KeyboardController::keys[button].held = false;
        return;
    }
#endif
    if(action == GLFW_PRESS) {
        KeyboardController::keys[button].pressed = true;
        KeyboardController::keys[button].held = true;
    } else if(action == GLFW_RELEASE) {
        KeyboardController::keys[button].released = true;
        KeyboardController::keys[button].held = false;
    }
    KeyboardController::keys[button].mod_flags = mods;
}
void KeyboardController::initCallbacks(GLFWwindow *window)
{
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);
}
KeyboardController::KeyboardController(GLFWwindow *window)
{
    //  means you are the first keyboard controller, so initialize
    if(keys.size() == 0) {
        KeyboardController::initCallbacks(window);
    }

    //  glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_WindowFocusCallback);
    //  glfwSetCursorEnterCallback(window, ImGui_ImplGlfw_CursorEnterCallback);
    //  glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
    //  glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    //  glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    //  glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
    //  glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    //  glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
}
void KeyboardController::update(Window &window, const Transform &camera_transform)
{
    m_scroll_velocity = scroll_velocity;
    scroll_velocity = { 0, 0 };

    double xpos, ypos;
    glfwGetCursorPos(window.getGLFWwindow(), &xpos, &ypos);
    xpos -= window.getSize().width / 2.f;
    ypos -= window.getSize().height / 2.f;
    m_mouse_pos = { xpos, ypos };
    m_global_mouse_pos = transformPoint(camera_transform.global(), m_mouse_pos);

    m_key_states.clear();
    for(auto binding : m_mappings) {
        const auto &state = keys[binding.second];

        m_key_states[binding.first] = state;
    }
    for(auto &[code, k] : keys) {
        k.released = false;
        k.pressed = false;
    }
}
vec2f KeyboardController::movementInPlane2D()
{
    glm::vec3 moveDir { 0.f };
    vec3f upDir { 0.f, -1.f, 0.f };
    vec3f rightDir { 1.f, 0.f, 0.f };
    if(m_key_states[eKeyMappings::MoveRight].held) {
        moveDir += rightDir;
    }
    if(m_key_states[eKeyMappings::MoveLeft].held) {
        moveDir -= rightDir;
    }
    if(m_key_states[eKeyMappings::MoveUp].held) {
        moveDir += upDir;
    }
    if(m_key_states[eKeyMappings::MoveDown].held) {
        moveDir -= upDir;
    }
    if(moveDir == glm::vec3(0.f)) {
        return moveDir;
    }
    return glm::normalize(moveDir);
}
vec2f KeyboardController::lookingInPlane2D()
{
    glm::vec3 moveDir { 0.f };
    vec3f upDir { 0.f, -1.f, 0.f };
    vec3f rightDir { -1.f, 0.f, 0.f };
    if(m_key_states[eKeyMappings::LookRight].held) {
        moveDir += rightDir;
    }
    if(m_key_states[eKeyMappings::LookLeft].held) {
        moveDir -= rightDir;
    }
    if(m_key_states[eKeyMappings::LookUp].held) {
        moveDir += upDir;
    }
    if(m_key_states[eKeyMappings::LookDown].held) {
        moveDir -= upDir;
    }
    if(moveDir == glm::vec3(0.f)) {
        return moveDir;
    }
    return glm::normalize(moveDir);
}
}  //  namespace emp
