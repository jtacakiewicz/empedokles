#include "log_window.hpp"
#include "debug/log.hpp"
namespace emp {
LogWindow::LogWindow()
{
    EMP_LOG(LogLevel::INFO) << "GUI log window started capturing logs...";
    Log::addLoggingOutput(std::move(std::make_unique<LogToGUIWindow>(this)));
}
static const ImVec4 ImGUILogLevelColors[] = {
    ImVec4(255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f),     ImVec4(255 / 255.0f, 255 / 255.0f, 0 / 255.0f, 1.0f),
    ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f), ImVec4(0 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f),
    ImVec4(0 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f),   ImVec4(0 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f),
    ImVec4(0 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f),   ImVec4(0 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f),
    ImVec4(0 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f)
};
static ImVec4 getImVecColor(LogLevel level)
{
    return ImGUILogLevelColors[level];
}
void LogWindow::draw(const char *title)
{
    if(!isOpen) {
        return;
    }

    if(!ImGui::Begin(title, &isOpen)) {
        ImGui::End();
        return;
    }

    //  As a specific feature guaranteed by the library, after calling
    //  Begin() the last Item represent the title bar. So e.g.
    //  IsItemHovered() will return true when hovering the title bar. Here we
    //  create a context menu only available from the title bar.

    if(ImGui::SmallButton("Clear")) {
        clearLog();
    }
    ImGui::SameLine();
    bool copy_to_clipboard = ImGui::SmallButton("Copy");
    //  static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t =
    //  ImGui::GetTime(); AddLog("Spam %f", t); }

    ImGui::Separator();

    //  Options menu
    if(ImGui::BeginPopup("Options")) {
        ImGui::Checkbox("Auto-scroll", &m_isAutoScrolling);
        ImGui::EndPopup();
    }

    //  Options, Filter
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_Tooltip);
    if(ImGui::Button("Options")) {
        ImGui::OpenPopup("Options");
    }
    ImGui::SameLine();
    m_filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
    ImGui::SetItemDefaultFocus();
    ImGui::Separator();

    //  Reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if(ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened,
                         ImGuiWindowFlags_HorizontalScrollbar)) {
        if(ImGui::BeginPopupContextWindow()) {
            if(ImGui::Selectable("Clear")) {
                clearLog();
            }
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
        if(copy_to_clipboard) {
            ImGui::LogToClipboard();
        }
        for(int i = 0; i < m_items.size(); i++) {
            auto &item = m_items[i];
            //  TODO
            if(!m_filter.PassFilter(item.first.c_str())) {
                continue;
            }

            //  Normally you would store more information in your item than
            //  just a string. (e.g. make Items[] an array of structure,
            //  store color/type etc.)
            ImVec4 color = getImVecColor(item.second);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(item.first.c_str());
            ImGui::PopStyleColor();
        }
        if(copy_to_clipboard) {
            ImGui::LogFinish();
        }

        //  Keep up at the bottom of the scroll region if we were already at
        //  the bottom at the beginning of the frame. Using a scrollbar or
        //  mouse-wheel will take away from the bottom edge.
        if(m_isScrollingToBottom || (m_isAutoScrolling && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
            ImGui::SetScrollHereY(1.0f);
        }
        m_isScrollingToBottom = false;

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();

    ImGui::End();
}
void LogWindow::clearLog()
{
    m_items.clear();
}
void LogWindow::addLog(LogLevel level, const char *fmt, ...)
{
    //  FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    m_items.push_back({ buf, level });
}
}
