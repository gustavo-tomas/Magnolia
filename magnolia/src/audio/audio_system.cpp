#include "magnolia/audio/audio_system.hpp"

#include <set>

#include "magnolia/resources/audio.hpp"

// NOLINTBEGIN
#include "soloud/include/soloud.h"
#include "soloud/include/soloud_wav.h"
// NOLINTEND

namespace mag
{
    namespace audio
    {
        // Private state
        struct State
        {
                std::set<SoLoud::Wav*> audios;
                SoLoud::Soloud soloud;
        };

        static State* state = nullptr;

        b8 initialize()
        {
            state = new State();

            // Initialize SoLoud (automatic back-end selection)
            const b8 result = state->soloud.init() == 0;

            return result;
        }

        void shutdown()
        {
            // Cleanup soloud
            for (const SoLoud::Wav* audio : state->audios)
            {
                delete audio;
            }

            state->soloud.deinit();

            delete state;
        }

        void register_audio_source(void* source)
        {
            auto* soloud_source = static_cast<SoLoud::Wav*>(source);

            if (state->audios.contains(soloud_source))
            {
                LOG_WARNING("Audio source is already loaded");
                return;
            }

            state->audios.insert(soloud_source);
        }

        void play(ref<AudioResource>& audio, const f32 volume, const math::vec3& position, const math::vec3& velocity)
        {
            audio->handle = state->soloud.play3d(*static_cast<SoLoud::Wav*>(audio->source), position.x, position.y,
                                                 position.z, velocity.x, velocity.y, velocity.z, volume);
        }

        void stop(const ref<AudioResource>& audio) { state->soloud.stop(audio->handle); }
    };  // namespace audio
};  // namespace mag
