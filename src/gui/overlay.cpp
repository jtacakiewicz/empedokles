#include "overlay.hpp"
namespace emp {
void Overlay::beginOverlay()
{
    ImGuiIO &io = ImGui::GetIO();
    if(location >= 0) {
        const float PAD = 10.0f;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;  //  Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    } else if(location == -2) {
        //  Center window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f);  //  Transparent background
    ImGui::Begin("simple overlay", &isOpen, window_flags);
}
void Overlay::endOverlay()
{
    ImGuiIO &io = ImGui::GetIO();
    if(ImGui::BeginPopupContextWindow()) {
        if(ImGui::MenuItem("Custom", NULL, location == -1)) {
            location = -1;
        }
        if(ImGui::MenuItem("Center", NULL, location == -2)) {
            location = -2;
        }
        if(ImGui::MenuItem("Top-left", NULL, location == 0)) {
            location = 0;
        }
        if(ImGui::MenuItem("Top-right", NULL, location == 1)) {
            location = 1;
        }
        if(ImGui::MenuItem("Bottom-left", NULL, location == 2)) {
            location = 2;
        }
        if(ImGui::MenuItem("Bottom-right", NULL, location == 3)) {
            location = 3;
        }
        if(ImGui::MenuItem("Close")) {
            isOpen = false;
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
void Overlay::draw(std::function<void(void)> drawFunc)
{
    if(!isOpen) {
        return;
    }
    beginOverlay();
    drawFunc();
    endOverlay();
}
};
