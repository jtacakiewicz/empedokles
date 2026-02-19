#include "sound_system.hpp"
namespace emp {
void SoundSystem::update()
{
    for(auto e : entities) {
        auto source = *ECS().getComponent<SoundSource>(e);
        auto transform = *ECS().getComponent<Transform>(e);
        auto pos = transform.getGlobalPosition();
        vec3f audio_position = { pos.x, pos.y, 0.f };
        source.setPosition(audio_position);
    }
}
};
