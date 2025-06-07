#include "resources/audio.hpp"

#include <map>

#include "resources/resource.hpp"
#include "resources/resource_loader.hpp"
#include "soloud/include/soloud_wav.h"

// @TODO: async loading

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<AudioResource>> audios;
                ResourceLoadedCallbackFn on_audio_loaded;
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

        ref<AudioResource> get_audio(const str& name)
        {
            auto it = state->audios.find(name);
            if (it != state->audios.end())
            {
                return it->second;
            }

            // Create a new audio
            AudioResource* audio = new AudioResource();
            audio->loading_status = LoadingStatus::InProgress;
            state->audios[name] = ref<AudioResource>(audio);

            if (resource::load(name, audio))
            {
                audio->loading_status = LoadingStatus::Finished;
                state->on_audio_loaded(audio);
            }

            else
            {
                audio->loading_status = LoadingStatus::Error;
            }

            return state->audios[name];
        }

        void set_on_audio_loaded_callback(const ResourceLoadedCallbackFn& callback)
        {
            state->on_audio_loaded = callback;
        }
    };  // namespace resource
};      // namespace mag
