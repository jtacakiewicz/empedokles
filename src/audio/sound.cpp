#include <al.h>
#include <AL/alc.h>

#include "sound.hpp"
#include "debug/log.hpp"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

namespace emp {
void SoundAsset::loadWav(const char *filename)
{
    drwav wav;
    if(!drwav_init_file(&wav, filename, nullptr)) {
        EMP_LOG(ERROR) << "Failed to open WAV file: " << filename;
        return;
    }
    m_pcm_buffer = std::vector<int16_t>(static_cast<size_t>(wav.totalPCMFrameCount) * wav.channels);
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, m_pcm_buffer.data());
    m_num_channels = wav.channels;
    m_sample_rate = wav.sampleRate;
    if(wav.channels == 1) {
        m_format = AL_FORMAT_MONO16;
    } else if(wav.channels == 2) {
        m_format = AL_FORMAT_STEREO16;
    } else {
        EMP_LOG(ERROR) << "Unsupported channel count: " << wav.channels;
    }
    drwav_uninit(&wav);
}
void SoundAsset::loadMp3(const char *filename)
{
    drmp3 mp3;
    if(!drmp3_init_file(&mp3, filename, nullptr)) {
        EMP_LOG(ERROR) << "Failed to open WAV file: " << filename;
        return;
    }
    m_pcm_buffer = std::vector<int16_t>(static_cast<size_t>(mp3.totalPCMFrameCount) * mp3.channels);
    drmp3_read_pcm_frames_s16(&mp3, mp3.totalPCMFrameCount, m_pcm_buffer.data());
    m_num_channels = mp3.channels;
    m_sample_rate = mp3.sampleRate;
    if(mp3.channels == 1) {
        m_format = AL_FORMAT_MONO16;
    } else if(mp3.channels == 2) {
        m_format = AL_FORMAT_STEREO16;
    } else {
        EMP_LOG(ERROR) << "Unsupported channel count: " << mp3.channels;
    }
    drmp3_uninit(&mp3);
}
void SoundAsset::loadFlac(const char *filename)
{
    drflac *flac = drflac_open_file(filename, nullptr);
    if(!flac) {
        EMP_LOG(ERROR) << "Failed to open FLAC file: " << filename;
        return;
    }
    m_pcm_buffer = std::vector<int16_t>(static_cast<size_t>(flac->totalPCMFrameCount) * flac->channels);
    drflac_read_pcm_frames_s16(flac, flac->totalPCMFrameCount, m_pcm_buffer.data());
    m_num_channels = flac->channels;
    m_sample_rate = flac->sampleRate;
    if(flac->channels == 1) {
        m_format = AL_FORMAT_MONO16;
    } else if(flac->channels == 2) {
        m_format = AL_FORMAT_STEREO16;
    } else {
        EMP_LOG(ERROR) << "Unsupported channel count: " << flac->channels;
    }
    drflac_close(flac);
}

SoundAsset::SoundAsset(std::string filename, AudioFileFormat format)
{
    if(format == AudioFileFormat::NOTSET) {
        auto suffix_pos = filename.rfind('.');
        auto suffix = filename.substr(suffix_pos + 1);
        if(suffix == "mp3") {
            format = AudioFileFormat::MP3;
        } else if(suffix == "wav") {
            format = AudioFileFormat::WAV;
        } else if(suffix == "flac") {
            format = AudioFileFormat::FLAC;
        } else {
            EMP_LOG(ERROR) << "unrecognized sound asset file format";
        }
    }

    switch(format) {
        case AudioFileFormat::MP3:
            loadMp3(filename.c_str());
            break;
        case AudioFileFormat::FLAC:
            loadFlac(filename.c_str());
            break;
        case AudioFileFormat::WAV:
            loadWav(filename.c_str());
            break;
        default:
            break;
    }
    alGenBuffers(1, &m_buffer);
    alBufferData(m_buffer, m_format, pcmBuffer().data(), static_cast<ALsizei>(pcmBuffer().size() * sizeof(int16_t)),
                 static_cast<ALsizei>(sample_rate()));
}
SoundAsset::~SoundAsset()
{
    alDeleteBuffers(1, &m_buffer);
}

AssetRegistry<SoundAsset> SoundSource::m_assets;

void checkError()
{
    ALCenum error = alGetError();
    if(error != AL_NO_ERROR) {
        EMP_LOG(ERROR) << "Failed to play sound: " << error;
    }
}
void SoundSource::create(std::string id, std::string filename, AudioFileFormat format)
{
    m_assets.create(id, filename, format);
}
SoundSource::SoundSource(std::string name)
{
    auto &sound_asset = m_assets.get(name);

    alSourcei(m_source, AL_BUFFER, sound_asset.alBuffer());
}

void SoundSource::setRolloff(float rolloff)
{
    alSourcef(m_source, AL_ROLLOFF_FACTOR, rolloff);
    checkError();
}
void SoundSource::setReferenceDistance(float ref_distance)
{
    alSourcef(m_source, AL_REFERENCE_DISTANCE, ref_distance);
    checkError();
}
void SoundSource::setMaxDistance(float max_distance)
{
    alSourcef(m_source, AL_MAX_DISTANCE, max_distance);
    checkError();
}
void SoundSource::play()
{
    alSourcePlay(m_source);
    checkError();
}
void SoundSource::setPosition(vec3f pos)
{
    alSource3f(m_source, AL_POSITION, pos.x, pos.y, pos.z);
    checkError();
}
}
