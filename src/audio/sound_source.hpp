#ifndef EMP_SOUND_SOURCE_HPP
#define EMP_SOUND_SOURCE_HPP
#include "audio/sound_asset.hpp"
#include "core/asset_registry.hpp"
#include "math/math_defs.hpp"
#include <al.h>
namespace emp {
struct SoundSystem;

#define EMP_MAX_SOUND_SOURCES 1024U
class SoundSource {
private:
    static AssetRegistry<SoundAsset> m_assets;

    void setPosition(vec3f pos);

    ALint m_source;

public:
    SoundSource() { }
    SoundSource(std::string name);
    static void create(std::string id, std::string filename, AudioFileFormat format = AudioFileFormat::NOTSET);

    void setRolloff(float rolloff);
    void setReferenceDistance(float ref_distance);
    void setMaxDistance(float max_distance);
    void play();
    ALint state() const
    {
        ALint state;
        alGetSourcei(m_source, AL_SOURCE_STATE, &state);
        return state;
    }
    bool isPlaying() const { return state() == AL_PLAYING; }
    bool isPaused() const { return state() == AL_PAUSED; }
    bool isStopped() const { return state() == AL_STOPPED; }

    friend SoundSystem;
};

};
#endif
