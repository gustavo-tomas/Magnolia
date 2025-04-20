#include "resources/audio.hpp"

#include "resources/resource_loader.hpp"
#include "soloud/include/soloud.h"
#include "soloud/include/soloud_wav.h"

// @TODO: async loading

namespace mag
{
    // Private implementation
    static SoLoud::Soloud soloud;

    AudioManager::AudioManager()
    {
        // Initialize SoLoud (automatic back-end selection)
        soloud.init();
    }

    AudioManager::~AudioManager()
    {
        for (auto& [name, audio] : audios)
        {
            delete static_cast<SoLoud::Wav*>(audio->source);
        }

        // Clean up SoLoud
        soloud.deinit();
    }

    ref<Audio> AudioManager::get(const str& name)
    {
        auto it = audios.find(name);
        if (it != audios.end())
        {
            return it->second;
        }

        // Create a new audio
        Audio* audio = new Audio();
        audios[name] = ref<Audio>(audio);

        resource::load(name, audio);

        return audios[name];
    }

    void AudioManager::play(ref<Audio>& audio, const f32 volume, const math::vec3& position, const math::vec3& velocity)
    {
        // This returns a handle
        audio->handle = soloud.play3d(*static_cast<SoLoud::Wav*>(audio->source), position.x, position.y, position.z,
                                      velocity.x, velocity.y, velocity.z, volume);
    }
};  // namespace mag
