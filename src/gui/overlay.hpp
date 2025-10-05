
#include "imgui.h"
#include <functional>
namespace emp {
class Overlay {
public:
    bool isOpen = true;

private:
    int location = 0;
    ImGuiWindowFlags window_flags;
    void beginOverlay();
    void endOverlay();

public:
    void draw(std::function<void(void)> drawFunc);
    Overlay()
    {
        window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                       ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    }
};

}
