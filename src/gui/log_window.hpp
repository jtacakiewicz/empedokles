#ifndef EMP_LOG_WINDOW
#define EMP_LOG_WINDOW
#include "imgui.h"
#include "debug/log.hpp"
namespace emp {

class LogWindow {
    std::vector<std::pair<std::string, LogLevel>> m_items;
    ImGuiTextFilter m_filter;
    bool m_isAutoScrolling = true;
    bool m_isScrollingToBottom = true;

public:
    bool isOpen = true;
    void draw(const char *title);
    void clearLog();
    void addLog(LogLevel level, const char *fmt, ...) IM_FMTARGS(3);
    LogWindow();
    LogWindow(const LogWindow &) = delete;
    LogWindow(LogWindow &&) = delete;
    LogWindow &operator=(const LogWindow &) = delete;
    LogWindow &operator=(LogWindow &&) = delete;
};

struct LogToGUIWindow : public LogOutput {
    LogWindow *console;
    void Output(std::string msg, LogLevel level) override
    {
        auto level_str = Log::ToString(level);
        console->addLog(level, "[%s] %s", level_str.c_str(), msg.c_str());
    }
    LogToGUIWindow(LogWindow *window)
        : console(window)
    {
    }
};
}
#endif
