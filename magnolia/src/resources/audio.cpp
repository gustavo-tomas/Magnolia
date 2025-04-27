#include "resources/audio.hpp"

#include <map>

#include "resources/resource_loader.hpp"
#include "soloud/include/soloud_wav.h"

// @TODO: async loading

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<Audio>> audios;
        };

        static State* state = nullptr;

        b8 initialize_audio_subsystem()
        {
            state = new State();

            return state != nullptr;
        }

        void shutdown_audio_subsystem()
        {
            for (auto& [name, audio] : state->audios)
            {
                delete static_cast<SoLoud::Wav*>(audio->source);
            }

            state->audios.clear();

            delete state;
        }

        ref<Audio> get_audio(const str& name)
        {
            auto it = state->audios.find(name);
            if (it != state->audios.end())
            {
                return it->second;
            }

            // Create a new audio
            Audio* audio = new Audio();
            state->audios[name] = ref<Audio>(audio);

            resource::load(name, audio);

            return state->audios[name];
        }
    };  // namespace resource
};      // namespace mag
