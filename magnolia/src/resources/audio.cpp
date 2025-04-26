#include "resources/audio.hpp"

#include "resources/resource_loader.hpp"
#include "soloud/include/soloud_wav.h"

// @TODO: async loading

namespace mag
{
    AudioManager::AudioManager() {}

    AudioManager::~AudioManager()
    {
        for (auto& [name, audio] : audios)
        {
            delete static_cast<SoLoud::Wav*>(audio->source);
        }
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
};  // namespace mag
