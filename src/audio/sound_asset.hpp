#ifndef EMP_SOUND_HPP
#define EMP_SOUND_HPP
#include "debug/log.hpp"
#include <AL/al.h>
#include <AL/alc.h>
namespace emp {

class SoundAsset {
    void loadMp3(const char *filename);
    void loadFlac(const char *filename);
    void loadWav(const char *filename);
    std::vector<int16_t> m_pcm_buffer;
    ALenum m_format;
    uint16_t m_num_channels;
    uint32_t m_sample_rate;

public:
    const std::vector<int16_t> &pcmBuffer() const { return m_pcm_buffer; }
    ALenum format() const { return m_format; }
    uint16_t num_channels() const { return m_num_channels; }
    uint32_t sample_rate() const { return m_sample_rate; }

    enum class AudioFileFormat { MP3, WAV, FLAC, NOTSET };
    SoundAsset(std::string filename, AudioFileFormat format = AudioFileFormat::NOTSET);
};

};
#endif
