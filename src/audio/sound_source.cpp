#include "sound_source.hpp"
#include <al.h>
namespace emp {
AssetRegistry<SoundAsset> SoundSource::m_assets;
ALuint SoundSource::handles[EMP_MAX_SOUND_SOURCES] { 0 };
size_t SoundSource::next_free_handle = 0U;

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
void SoundSource::init()
{
    alGenSources(EMP_MAX_SOUND_SOURCES, handles);
    next_free_handle = 0;
}
void SoundSource::deinit()
{
    alDeleteSources(EMP_MAX_SOUND_SOURCES, handles);
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
