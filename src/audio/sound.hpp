#ifndef EMP_SOUND_HPP
#define EMP_SOUND_HPP
#include <al.h>
#include <AL/alc.h>

#include "core/asset_registry.hpp"
#include "math/math_defs.hpp"
#include "debug/log.hpp"

namespace emp {
struct SoundSystem;

enum class AudioFileFormat { MP3, WAV, FLAC, NOTSET };
class SoundAsset {
    void loadMp3(const char *filename);
    void loadFlac(const char *filename);
    void loadWav(const char *filename);
    std::vector<int16_t> m_pcm_buffer;
    ALenum m_format;
    uint16_t m_num_channels;
    uint32_t m_sample_rate;
    ALuint m_buffer;

public:
    const std::vector<int16_t> &pcmBuffer() const { return m_pcm_buffer; }
    const ALuint alBuffer() const { return m_buffer; }
    ALenum format() const { return m_format; }
    uint16_t num_channels() const { return m_num_channels; }
    uint32_t sample_rate() const { return m_sample_rate; }

    SoundAsset(std::string filename, AudioFileFormat format = AudioFileFormat::NOTSET);
    ~SoundAsset();
};

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
