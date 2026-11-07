#ifndef EMP_SOUND_SYSTEM_HPP
#define EMP_SOUND_SYSTEM_HPP
#include "audio/sound_asset.hpp"
#include "audio/sound_source.hpp"
#include "scene/transform.hpp"
#include "core/system.hpp"
#include <alc.h>
#include <string>
namespace emp {
class SoundSystem : public System<Transform, SoundSource> {
    ALCdevice *m_device;
    ALCcontext *m_context;

public:
    SoundSystem();
    void update();
};

};
#endif
