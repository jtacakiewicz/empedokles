#ifndef EMP_SOUND_SYSTEM_HPP
#define EMP_SOUND_SYSTEM_HPP
#include "audio/sound.hpp"
#include "scene/transform.hpp"
#include "core/system.hpp"
#include <alc.h>
#include <string>
namespace emp {
class SoundSystem : public System<Transform, SoundSource> {
    ALCdevice *m_device;
    ALCcontext *m_context;
    ALuint m_handles[EMP_MAX_SOUND_SOURCES];
    std::vector<ALuint> m_free_handles;

    void onEntityRemoved(Entity entity) override;
    void onEntityAdded(Entity entity) override;
public:
    SoundSystem();
    void update();
};

};
#endif
