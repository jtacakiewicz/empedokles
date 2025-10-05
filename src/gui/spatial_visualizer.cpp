#include "spatial_visualizer.hpp"
#include "imgui.h"
#include "core/coordinator.hpp"
#include "core/system.hpp"
#include "graphics/camera.hpp"
#include "math/math_func.hpp"
#include "physics/collider.hpp"
#include "scene/transform.hpp"
namespace emp {
void SpatialVisualizer::draw(const char *title, Coordinator &ECS, std::function<std::string(Entity)> namingFunc, Camera &camera)
{
    if(!isOpen) {
        return;
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.f, 2.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.f, 0.f));
    //  Create a window with no background
    ECS.getSystem<TransformSystem>()->performDFS([&](Entity entity, Transform &transform) {
        AABB window;
        auto collider = ECS.getComponent<Collider>(entity);
        auto entity_name = namingFunc(entity);
        if(collider) {
            window = collider->extent();
            window = AABB::TransformedAABB(transform.global(), window);
        } else {
            ImVec2 textSize = ImGui::CalcTextSize(entity_name.c_str());
            const ImGuiStyle &style = ImGui::GetStyle();
            float reqWidth = textSize.x + style.WindowPadding.x * 2.0f;
            auto pos = transformPoint(transform.global(), vec2f(10, 10));
            window = AABB::CreateMinMax(pos, pos + vec2f(reqWidth, 0.f));
        }
        auto center = camera.convertWorldToScreenPosition(window.bl());
        ImVec2 size = { window.size().x, window.size().y };
        ImVec2 position = { center.x, center.y };
        ImGui::SetNextWindowPos(position);  //  Apply position
        ImGui::SetNextWindowSize(size);

        ImGui::Begin(entity_name.c_str(), nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMouseInputs);
        ImGui::End();
    });
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}
}
