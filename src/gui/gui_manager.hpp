#ifndef EMP_GUI_MANAGER_HPP
#define EMP_GUI_MANAGER_HPP

#include "core/coordinator.hpp"
#include "gui/console.hpp"
#include "gui/editor/inspector.hpp"
#include "gui/editor/tree-view.hpp"
#include "gui/log_window.hpp"
#include "gui/overlay.hpp"
#include "gui/spatial_visualizer.hpp"

namespace emp {

class GUIManager {
    bool m_showDemoWindow = false;

    Entity m_entity_selected;
    std::mutex m_access_mutex;
    std::map<Entity, std::string> m_names_aliased;
    std::set<std::string> m_names_used;

    Overlay m_FPS_overlay;
    static constexpr int TIME_SAMPLE_COUNT = 120;
    int renderer_time_idx = 0;
    int physics_time_idx = 0;
    int mainUpdate_time_idx = 0;
    float renderer_time[TIME_SAMPLE_COUNT] { 0 };
    float physics_time[TIME_SAMPLE_COUNT] { 0 };
    float mainUpdate_time[TIME_SAMPLE_COUNT] { 0 };

    TreeView m_tree_view;
    Inspector m_inspector;
    bool *inspectorMaster = nullptr;
    Console m_console;
    LogWindow m_log_window;
    SpatialVisualizer m_visualizer;

    void drawMainMenuBar();
    void drawFPSOverlay();

public:
    void addRendererTime(float time);
    void addPhysicsTime(float time);
    void addUpdateTime(float time);

    void alias(Entity entity, std::string name);
    void draw(Coordinator &, Camera &);
    GUIManager();
};

}

#endif  //  !DEBUG
