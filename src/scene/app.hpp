#ifndef EMP_APP_HPP
#define EMP_APP_HPP
#include "core/coordinator.hpp"
#include "graphics/camera.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/render_systems/simple_render_system.hpp"
#include "graphics/renderer.hpp"
#include "gui/gui_manager.hpp"
#include "scene/renderer_context.hpp"
#include "vulkan/device.hpp"

#include "io/keyboard_controller.hpp"
#include "io/window.hpp"

namespace emp {
class App {
public:
    struct AssetInfo {
        std::string filename;
        const char *id;
    };

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

    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }

    virtual void onUpdate(const float, Window &, KeyboardController &) { }
    virtual void onFixedUpdate(const float, Window &, KeyboardController &) { }
    virtual void onRender(Device &, const FrameInfo &) { }
    virtual void onSetup(Window &, Device &) { }

private:
    float m_width = 800;
    float m_height = 800;

    std::atomic<bool> isAppRunning = true;

    RendererContext renderer_context;

    std::vector<AssetInfo> m_models_to_load;
    std::vector<AssetInfo> m_textures_to_load;

    void setupECS();

    void m_updateUBO(float deltaTime, Camera &camera, Buffer &uboBuffer, Buffer &computeUboBuffer);
    void loadAssets();

    void renderFrame(Camera &camera, RendererContext &render_contex, float delta_time);
};
}  //  namespace emp
#endif
