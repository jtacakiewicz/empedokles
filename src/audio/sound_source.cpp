#include "sound_source.hpp"
#include <al.h>
namespace emp {
AssetRegistry<SoundAsset> SoundSource::m_assets;
SoundSource SoundSource::create(std::string id, std::string filename, AudioFileFormat format)
{
    m_assets.create(id, filename, format);
    return SoundSource(m_assets.get(id));
}
SoundSource::SoundSource(const SoundAsset &sound_asset)
{
    alGenBuffers(1, &m_buffer);

    alBufferData(m_buffer, sound_asset.format(), sound_asset.pcmBuffer().data(),
                 static_cast<ALsizei>(sound_asset.pcmBuffer().size() * sizeof(int16_t)),
                 static_cast<ALsizei>(sound_asset.sample_rate()));

    alGenSources(1, &m_source);
    alSourcei(m_source, AL_BUFFER, m_buffer);
}
SoundSource::~SoundSource()
{
    alDeleteSources(1, &m_source);
    alDeleteBuffers(1, &m_buffer);
}
void SoundSource::setRolloff(float rolloff)
{
    alSourcef(m_source, AL_ROLLOFF_FACTOR, rolloff);
}
void SoundSource::setReferenceDistance(float ref_distance)
{
    alSourcef(m_source, AL_REFERENCE_DISTANCE, ref_distance);
}
void SoundSource::setMaxDistance(float max_distance)
{
    alSourcef(m_source, AL_MAX_DISTANCE, max_distance);
}
void SoundSource::play()
{
    alSourcePlay(m_source);
}
void SoundSource::setPosition(vec3f pos)
{
    alSource3f(id(), AL_POSITION, pos.x, pos.y, pos.z);
}
}
