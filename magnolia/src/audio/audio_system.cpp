#include "magnolia/audio/audio_system.hpp"

#include "magnolia/resources/audio.hpp"
#include "soloud/include/soloud.h"
#include "soloud/include/soloud_wav.h"

namespace mag
{
    namespace audio
    {
        // Private state
        struct State
        {
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
            state->soloud.deinit();

            delete state;
        }

        void play(ref<AudioResource>& audio, const f32 volume, const math::vec3& position, const math::vec3& velocity)
        {
            audio->handle = state->soloud.play3d(*static_cast<SoLoud::Wav*>(audio->source), position.x, position.y,
                                                 position.z, velocity.x, velocity.y, velocity.z, volume);
        }

        void stop(const ref<AudioResource>& audio) { state->soloud.stop(audio->handle); }
    };  // namespace audio
};  // namespace mag
