#ifndef EMP_CONSOLE_HPP
#define EMP_CONSOLE_HPP
#include <functional>
#include <map>
#include "imgui.h"
#include "core/coordinator.hpp"
namespace emp {
class Console {
public:
    struct Command {
        virtual std::string expression() = 0;
        virtual std::string description() { return ""; }
        virtual std::string exec(const std::vector<std::string> &arguments, const std::set<std::string> &boolFlags,
                                 const std::map<std::string, std::string> &valueFlags) = 0;
        virtual ~Command() { }
    };

private:
    std::string m_input_buf;
    std::vector<std::string> m_items;
    std::vector<std::unique_ptr<Command>> m_commands;
    std::vector<std::string> m_history;
    int m_history_pos;  //  -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter m_filter;
    bool m_isAutoScrolling;
    bool m_isScrollingToBottom;
    typedef std::function<int(ImGuiInputTextCallbackData *)> Callback_t;
    std::map<ImGuiInputTextFlags, Callback_t> m_callbacks;

    void execCommand(std::string &command_line);
    static int TextEditCallbackStub(ImGuiInputTextCallbackData *data);
    void registerCallbacks();
    int processTextInput();

public:
    bool isOpen = true;
    Console();
    ~Console();

    void addCommand(std::unique_ptr<Command> &&cmd);
    void clearLog();
    void addLog(const char *fmt, ...) IM_FMTARGS(2);
    void draw(const char *title);
    std::vector<std::pair<std::string, std::string>> listCommands();

    Console(const Console &) = delete;
    Console(Console &&) = delete;
    Console &operator=(const Console &) = delete;
    Console &operator=(Console &&) = delete;
};

}  //  namespace emp
#endif  //  !EMP_CONSOLE_HPP
