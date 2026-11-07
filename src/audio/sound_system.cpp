#include "sound_system.hpp"
#include "audio/sound_source.hpp"
#include <cstring>
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
SoundSystem::SoundSystem()
{
    m_device = alcOpenDevice(NULL);
    m_context = alcCreateContext(m_device, NULL);
    if(!alcMakeContextCurrent(m_context)) {
        ALCenum error;

        error = alGetError();
        if(error != AL_NO_ERROR) {
            EMP_LOG(ERROR) << "failed to initialize sound system";
        }
    }
    SoundSource::init();
}
};
