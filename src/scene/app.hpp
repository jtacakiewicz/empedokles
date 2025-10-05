#ifndef EMP_APP_HPP
#define EMP_APP_HPP
#include "compute/compute_manager.hpp"
#include "core/coordinator.hpp"
#include "graphics/camera.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/model_system.hpp"
#include "graphics/renderer.hpp"
#include "graphics/render_systems/simple_render_system.hpp"
#include "gui/gui_manager.hpp"
#include "scene/compute_demo.hpp"
#include "scene/renderer_context.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"

#include <condition_variable>
#include <iostream>
#include <thread>
#include "io/keyboard_controller.hpp"
#include "io/window.hpp"
#include "math/math_defs.hpp"
#include "physics/physics_system.hpp"
namespace emp {
class App {
public:
    struct AssetInfo {
        std::string filename;
        const char *id;
    };

    virtual void onUpdate(const float, Window &, KeyboardController &) { }
    virtual void onFixedUpdate(const float, Window &, KeyboardController &) { }
    virtual void onRender(Device &, const FrameInfo &) { }
    virtual void onSetup(Window &, Device &) { }

    App(const int width, const int height, std::vector<AssetInfo> models_to_load, std::vector<AssetInfo> textures_to_load);
    virtual ~App();
    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();

protected:
    Window window;
    Device device;

    GUIManager gui_manager;
    Renderer renderer;
    //  order of declarations matters

    KeyboardController controller;
    Coordinator ECS;

    void setPhysicsTickrate(const float tick_rate);
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }

private:
    float m_width = 800;
    float m_height = 800;

    //  synchronization systems
    std::condition_variable m_priority_access;
    std::mutex m_coordinator_access_mutex;
    std::atomic<bool> m_isRenderer_waiting = false;
    std::atomic<bool> m_isPhysics_waiting = false;

    std::atomic<float> m_physics_tick_rate = INFINITY;
    std::atomic<bool> isAppRunning = true;

    RendererContext renderer_context;

    std::vector<AssetInfo> m_models_to_load;
    std::vector<AssetInfo> m_textures_to_load;

    void setupECS();

    void m_updateUBO(float deltaTime, Camera &camera, Buffer &uboBuffer, Buffer &computeUboBuffer);
    void loadAssets();

    void renderFrame(Camera &camera, RendererContext &render_contex, float delta_time);
    std::unique_ptr<std::thread> createRenderThread(Camera &camera);
    std::unique_ptr<std::thread> createPhysicsThread();
};
}  //  namespace emp
#endif
