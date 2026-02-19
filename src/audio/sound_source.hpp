#ifndef EMP_SOUND_SOURCE_HPP
#define EMP_SOUND_SOURCE_HPP
#include "audio/sound_asset.hpp"
#include "core/asset_registry.hpp"
#include "math/math_defs.hpp"
#include <al.h>
namespace emp {
struct SoundSystem;

class SoundSource {
private:
    static AssetRegistry<SoundAsset> m_assets;

    ALuint m_source = -1;
    ALuint m_buffer;

    void setPosition(vec3f pos);

public:
    SoundSource() { }
    SoundSource(const SoundAsset &sound_asset);
    static SoundSource create(std::string id, std::string filename, AudioFileFormat format = AudioFileFormat::NOTSET);

    ~SoundSource();
    void setRolloff(float rolloff);
    void setReferenceDistance(float ref_distance);
    void setMaxDistance(float max_distance);
    void play();
    ALuint id() const { return m_source; }
    ALint state() const
    {
        ALint state;
        alGetSourcei(id(), AL_SOURCE_STATE, &state);
        return state;
    }
    bool isPlaying() const { return state() == AL_PLAYING; }
    bool isPaused() const { return state() == AL_PAUSED; }
    bool isStopped() const { return state() == AL_STOPPED; }

    friend SoundSystem;
};

};
#endif
