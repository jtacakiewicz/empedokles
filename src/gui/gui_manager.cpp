#include "gui/gui_manager.hpp"
#include "imgui.h"
#include <mutex>
#include <string>
#include "debug/log.hpp"
#include "gui/editor/inspector.hpp"
#include "scene/scene_defs.hpp"
namespace emp {
void GUIManager::addRendererTime(float time)
{
    std::unique_lock<std::mutex> lock { m_access_mutex };
    renderer_time[renderer_time_idx++ % TIME_SAMPLE_COUNT] = time;
}
void GUIManager::addPhysicsTime(float time)
{
    std::unique_lock<std::mutex> lock { m_access_mutex };
    physics_time[physics_time_idx++ % TIME_SAMPLE_COUNT] = time;
}
void GUIManager::addUpdateTime(float time)
{
    std::unique_lock<std::mutex> lock { m_access_mutex };
    mainUpdate_time[mainUpdate_time_idx++ % TIME_SAMPLE_COUNT] = time;
}
void GUIManager::alias(Entity entity, std::string name)
{
    std::unique_lock<std::mutex> lock { m_access_mutex };
    auto input_name = name;
    int copy_number = 1;
    while(m_names_used.contains(name)) {
        name = input_name + '(' + std::to_string(copy_number++) + ')';
    }
    m_names_aliased[entity] = name;
    m_names_used.insert(name);
}

void GUIManager::drawMainMenuBar()
{
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Demo", NULL, m_showDemoWindow)) {
                m_showDemoWindow = !m_showDemoWindow;
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit")) {
            if(ImGui::MenuItem("Entity Tree View", NULL, m_tree_view.isOpen)) {
                m_tree_view.isOpen = !m_tree_view.isOpen;
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("View")) {
            if(ImGui::MenuItem("FPS Overlay", NULL, m_FPS_overlay.isOpen)) {
                m_FPS_overlay.isOpen = !m_FPS_overlay.isOpen;
            }
            if(ImGui::MenuItem("Show entities", NULL, m_visualizer.isOpen)) {
                m_visualizer.isOpen = !m_visualizer.isOpen;
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Tools")) {
            if(ImGui::MenuItem("Log Window", NULL, m_log_window.isOpen)) {
                m_log_window.isOpen = !m_log_window.isOpen;
            }
            if(ImGui::MenuItem("Console", NULL, m_console.isOpen)) {
                m_console.isOpen = !m_console.isOpen;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void GUIManager::draw(Coordinator &coordinator, Camera &camera)
{
    std::unique_lock<std::mutex> lock { m_access_mutex };
    drawMainMenuBar();
    if(m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    m_console.draw("Console");
    m_log_window.draw("Log Window");
    auto naming_function = [&](Entity e) {
        auto &map = m_names_aliased;
        return map.find(e) == map.end() ? "entity_" + std::to_string(e) : map.at(e);
    };
    m_tree_view.draw("Tree view of entities", coordinator, naming_function);

    if(m_tree_view.isJustSelected()) {
        m_inspector.isOpen = true;
        m_entity_selected = m_tree_view.getSelected();

        inspectorMaster = &m_tree_view.isOpen;
        m_inspector.isOpen = true;
    }
    if(inspectorMaster) {
        if(!m_inspector.isOpen || !*inspectorMaster) {
            *inspectorMaster = false;
            m_inspector.isOpen = false;
            inspectorMaster = nullptr;
        }
    }
    m_inspector.draw(m_tree_view.getSelected(), coordinator);
    drawFPSOverlay();
    m_visualizer.draw("visualizer", coordinator, naming_function, camera);
}
void GUIManager::drawFPSOverlay()
{
    m_FPS_overlay.draw([&]() {
        auto denom = 1.f / TIME_SAMPLE_COUNT;
        auto avg_render = std::reduce(renderer_time, renderer_time + TIME_SAMPLE_COUNT) * denom;
        auto avg_physics = std::reduce(physics_time, physics_time + TIME_SAMPLE_COUNT) * denom;
        auto avg_update = std::reduce(mainUpdate_time, mainUpdate_time + TIME_SAMPLE_COUNT) * denom;

        if(EMP_ENABLE_RENDER_THREAD) {
            ImGui::Text("renderer FPS: %.4g", 1.0 / avg_render);
        } else {
            ImGui::Text("render time: %.3g%%", floorf(avg_render / avg_update * 100.f));
        }
        if(EMP_ENABLE_PHYSICS_THREAD) {
            ImGui::Text("physics  TPS: %.4g", 1.0 / avg_physics);
        } else {
            ImGui::Text("physics  time: %.3g%%", floorf(avg_physics / avg_update * 100.f));
        }
        ImGui::Text("mainLoop TPS: %.4g", 1.0 / avg_update);
        float *FPS = mainUpdate_time;
        int FPS_idx = mainUpdate_time_idx;
        if(EMP_ENABLE_RENDER_THREAD) {
            FPS = renderer_time;
            FPS_idx = renderer_time_idx;
        }
        ImGui::PlotLines("##compute time", mainUpdate_time, TIME_SAMPLE_COUNT, mainUpdate_time_idx, nullptr, 0.0f, 0.05f,
                         ImVec2(0, 40.0f));
    });
}
GUIManager::GUIManager()
{
    m_inspector.isOpen = false;
    m_console.isOpen = false;
    m_log_window.isOpen = false;
    m_tree_view.isOpen = false;
    m_visualizer.isOpen = false;
}
}
