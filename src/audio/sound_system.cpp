#include "sound_system.hpp"
#include <cstring>
#include <algorithm>

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
void SoundSystem::onEntityRemoved(Entity entity) {
    auto& sound = this->getComponent<SoundSource>(entity);
    m_free_handles.push_back(sound.m_source);
}
void SoundSystem::onEntityAdded(Entity entity) {
    auto& sound = this->getComponent<SoundSource>(entity);
    auto handle = m_free_handles.back();
    m_free_handles.pop_back();

    sound.m_source = handle; //TODO
    auto &sound_asset = sound.m_assets.get(sound.m_name);
    alSourcei(handle, AL_BUFFER, sound_asset.alBuffer());
}
SoundSystem::SoundSystem() : m_free_handles(EMP_MAX_SOUND_SOURCES)
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
    alGenSources(EMP_MAX_SOUND_SOURCES, m_handles);
    ALCenum error = alGetError();
    if(error != AL_NO_ERROR) {
        EMP_LOG(ERROR) << "Failed to allocate sound handles: " << error;
    }
    for(int i = 0; i < EMP_MAX_SOUND_SOURCES; i++) {
        m_free_handles[i] = m_handles[i];
    }
}
};
