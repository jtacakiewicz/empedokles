//  based on ImGui Console example
#include "console.hpp"
#include "imgui.h"
#include <iterator>
#include <memory>
#include <sstream>
#include <tuple>
#include "debug/log.hpp"
namespace emp {
struct LogCommand : public Console::Command {
    std::string expression() override { return "log"; }
    std::string exec(const std::vector<std::string> &arguments, const std::set<std::string> &boolFlags,
                     const std::map<std::string, std::string> &valueFlags) override
    {
        std::string output;
        output += "args: ";
        for(auto o : arguments) {
            output += o + ", ";
        }
        output += "bools: ";
        for(auto o : boolFlags) {
            output += o + ", ";
        }
        output += "vals: ";
        for(auto [key, value] : valueFlags) {
            output += key + ":" + value + ", ";
        }
        return output;
    }
};
struct HelpCommand : public Console::Command {
    Console *console;
    std::string expression() override { return "help"; }
    std::string exec(const std::vector<std::string> &arguments, const std::set<std::string> &boolFlags,
                     const std::map<std::string, std::string> &valueFlags) override
    {
        if(arguments.size() != 0) {
            return "arguments unexpected!, type 'help' to see all commands";
        }
        if(boolFlags.size() != 0 || valueFlags.size() != 0) {
            return "flags unexpected!, type 'help' to see all commands";
        }
        std::string result = "Available commands:\n";
        for(int i = 0; i < console->listCommands().size(); i++) {
            auto command_info = console->listCommands()[i];
            auto expression = command_info.first;
            auto description = command_info.second;
            result += "\t- " + expression;
            if(description != "") {
                result += " - " + description;
            }
            result += "\n";
        }
        return result;
    }
    HelpCommand(Console *cons)
        : console(cons)
    {
    }
};
std::vector<std::pair<std::string, std::string>> Console::listCommands()
{
    std::vector<std::pair<std::string, std::string>> commands;
    for(int i = 0; i < m_commands.size(); i++) {
        auto cmd = m_commands[i].get();
        if(!cmd) {
            continue;
        }
        commands.push_back({ cmd->expression(), cmd->description() });
    }
    return commands;
}
Console::Console()
{
    clearLog();
    m_history_pos = -1;

    addCommand(std::make_unique<LogCommand>());
    addCommand(std::make_unique<HelpCommand>(this));

    //  "CLASSIFY" is here to provide the test case where "C"+[tab] completes
    //  to "CL" and display multiple matches.
    //  m_commands.push_back("HELP");
    //  m_commands.push_back("HISTORY");
    //  m_commands.push_back("CLEAR");
    //  m_commands.push_back("CLASSIFY");
    m_isAutoScrolling = true;
    m_isScrollingToBottom = false;
    registerCallbacks();
}
Console::~Console() { }

void Console::addCommand(std::unique_ptr<Console::Command> &&cmd)
{
    m_commands.push_back(std::move(cmd));
}
void Console::clearLog()
{
    m_items.clear();
}

void Console::addLog(const char *fmt, ...)
{
    //  FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    m_items.push_back(buf);
}
bool InputText(const char *label, std::string &str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = 0,
               void *user_data = nullptr)
{
    //  Allocate a temporary buffer large enough to hold the current string and potential edits
    char buffer[1024];

    //  Copy the std::string content into the buffer
    strncpy(buffer, str.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';  //  Ensure null-termination

    //  Call ImGui's InputText with the buffer
    bool modified = ImGui::InputText(label, buffer, sizeof(buffer), flags, callback, user_data);

    //  If the user modified the text, update the std::string
    if(modified) {
        str = buffer;
    }

    return modified;  //  Return true if the text was modified
}

void Console::draw(const char *title)
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

    ImGui::TextWrapped("Enter 'HELP' for help.");

    //  TODO: display items starting from the bottom

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
        ImGuiListClipper clipper;
        clipper.Begin(m_items.size());
        while(clipper.Step()) {
            for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                auto &item = m_items[i];
                //  TODO
                //   if (!m_filter.PassFilter(item.c_str()))
                //       continue;

                //  Normally you would store more information in your item than
                //  just a string. (e.g. make Items[] an array of structure,
                //  store color/type etc.)
                ImVec4 color;
                bool has_color = false;
                if(item.find("[error]") != std::string::npos) {
                    color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                    has_color = true;
                } else if(item.find("# ", 2) == 0) {
                    color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
                    has_color = true;
                }
                if(has_color) {
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                }
                ImGui::TextUnformatted(item.c_str());
                if(has_color) {
                    ImGui::PopStyleColor();
                }
            }
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
    ImGui::Separator();

    //  Command-line
    bool reclaim_focus = processTextInput();
    //  Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
    if(reclaim_focus) {
        ImGui::SetKeyboardFocusHere(-1);  //  Auto focus previous widget
    }

    ImGui::End();
}
int Console::processTextInput()
{
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll |
                                           ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
    if(InputText("Input", m_input_buf, input_text_flags, &TextEditCallbackStub, (void *)this)) {
        if(m_input_buf.size() > 0) {
            execCommand(m_input_buf);
        }
        m_input_buf = "";
        return 1;
    }
    return 0;
}

void Console::execCommand(std::string &command_line)
{
    m_isScrollingToBottom = true;
    addLog("> %s\n", command_line.c_str());

    //  Insert into history. First find match and delete it so it can be
    //  pushed to the back. This isn't trying to be smart or optimal.
    m_history_pos = -1;
    for(int i = m_history.size() - 1; i >= 0; i--) {
        if(m_history[i].find(command_line) == 0) {
            m_history.erase(m_history.begin() + i);
            break;
        }
    }
    m_history.push_back(command_line);

    std::stringstream ss(command_line);
    std::string expression;
    ss >> expression;
    Command *executable = nullptr;
    for(int i = 0; i < m_commands.size(); i++) {
        Command *cmd = m_commands[i].get();
        if(cmd != nullptr && cmd->expression() == expression) {
            executable = cmd;
        }
    }
    if(executable == nullptr) {
        addLog("Unknown command: '%s'\n", command_line.c_str());
        return;
    }
    //  convert space to delimiter
    //  this is needed to preserve spaces in quoutes, and use 'delim' as delimiter
    //  for stringstream
    //   command_line = ss.str();
    //   const char delim = '\x7';
    //   bool isStringStarted = false;
    //   bool ignoreNext = false;
    //   for(int i = 0; i < command_line.size(); i++) {
    //       if(!isStringStarted && (command_line[i] == ' ' || command_line[i] == '\t')) {
    //           command_line[i] = delim;
    //       }
    //       if(command_line[i] == '"' && !ignoreNext) {
    //           isStringStarted = !isStringStarted;
    //       }
    //       ignoreNext = false;
    //       if(command_line[i] == '\\') {
    //           ignoreNext = true;
    //       }
    //   }
    //   ss = std::stringstream(command_line);
    //
    std::vector<std::string> arguments;
    std::set<std::string> boolFlags;
    std::map<std::string, std::string> valueFlags;
    std::string cur_arg;

    auto cleanQuoutes = [](std::string in) {
        int frontal = in.find('"');
        int endal = in.rfind('"');
        while(frontal != endal && frontal != std::string::npos && endal != std::string::npos) {
            in.erase(frontal, 1);
            in.erase(endal - 1, 1);
            frontal = in.find('"');
            endal = in.rfind('"');
        }
        return in;
    };
    while(ss >> cur_arg) {
        cur_arg = cleanQuoutes(cur_arg);
        if(cur_arg.front() != '-') {
            arguments.push_back(cur_arg);
        } else if(cur_arg.find("=") == std::string::npos) {
            boolFlags.insert(cur_arg.substr(1));
        } else {
            int split = cur_arg.find("=");
            std::string key = cur_arg.substr(1, split - 1);
            std::string value = cur_arg.substr(split + 1);

            valueFlags[key] = value;
        }
    }
    std::string message = executable->exec(arguments, boolFlags, valueFlags);

    addLog("%s", message.c_str());
}

int Console::TextEditCallbackStub(ImGuiInputTextCallbackData *data)
{
    Console *console = (Console *)data->UserData;
    auto callback = console->m_callbacks.find(data->EventFlag);
    if(callback == console->m_callbacks.end()) {
        return 0;
    }
    return callback->second(data);
}

void Console::registerCallbacks()
{
    m_callbacks[ImGuiInputTextFlags_CallbackCompletion] = [&](ImGuiInputTextCallbackData *data) -> int {
        std::string input = data->Buf;
        int word_end = data->CursorPos;
        int word_start = word_end;
        while(word_start > 0) {
            const char c = input[word_start - 1];
            if(c == ' ' || c == '\t' || c == ',' || c == ';') {
                break;
            }
            word_start--;
        }

        std::vector<std::string> candidates;
        for(int i = 0; i < m_commands.size(); i++) {
            int location = m_commands[i]->expression().find(input.substr(word_start));
            if(location == 0) {
                candidates.push_back(m_commands[i]->expression());
            }
        }

        if(candidates.size() == 0) {
            addLog("No match for \"%s\"!\n", input.c_str());
        } else if(candidates.size() == 1) {
            data->DeleteChars(word_start, word_end - word_start);
            data->InsertChars(data->CursorPos, candidates[0].c_str());
            data->InsertChars(data->CursorPos, " ");
        } else {
            EMP_LOG_DEBUG << "complex";
            int match_len = word_end - word_start;
            for(;;) {
                int c = 0;
                bool all_candidates_matches = true;
                for(int i = 0; i < candidates.size() && all_candidates_matches; i++) {
                    if(i == 0) {
                        c = toupper(candidates[i][match_len]);
                    } else if(c == 0 || c != toupper(candidates[i][match_len])) {
                        all_candidates_matches = false;
                    }
                }
                if(!all_candidates_matches) {
                    break;
                }
                match_len++;
            }

            if(match_len > 0) {
                EMP_LOG_DEBUG << match_len;
                data->DeleteChars(word_start, word_end - word_start);
                data->InsertChars(data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
                EMP_LOG_DEBUG << data->Buf;
            }

            addLog("Possible matches:\n");
            for(int i = 0; i < candidates.size(); i++) {
                addLog("- %s\n", candidates[i].c_str());
            }
        }
        return 0;
    };
    m_callbacks[ImGuiInputTextFlags_CallbackHistory] = [&](ImGuiInputTextCallbackData *data) -> int {
        const int prev_history_pos = m_history_pos;
        if(data->EventKey == ImGuiKey_UpArrow) {
            if(m_history_pos == -1) {
                m_history_pos = m_history.size() - 1;
            } else if(m_history_pos > 0) {
                m_history_pos--;
            }
        } else if(data->EventKey == ImGuiKey_DownArrow) {
            if(m_history_pos != -1) {
                if(++m_history_pos >= m_history.size()) {
                    m_history_pos = -1;
                }
            }
        }

        if(prev_history_pos != m_history_pos) {
            std::string history_str = (m_history_pos >= 0) ? m_history[m_history_pos] : "";
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, history_str.c_str());
        }
        return 0;
    };
}
}
