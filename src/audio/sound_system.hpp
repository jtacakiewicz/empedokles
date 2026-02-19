#ifndef EMP_SOUND_SYSTEM_HPP
#define EMP_SOUND_SYSTEM_HPP
#include "audio/sound_asset.hpp"
#include "audio/sound_source.hpp"
#include "scene/transform.hpp"
#include "core/system.hpp"
#include <string>
namespace emp {
class SoundSystem : System<Transform, SoundSource> {
    std::unordered_map<std::string, SoundAsset> m_sounds;

public:
    void update();
    void createAsset(std::string id, std::string filename,
                     SoundAsset::AudioFileFormat format = SoundAsset::AudioFileFormat::NOTSET);
    SoundSource createSource(std::string asset_id);
};

};
#endif
