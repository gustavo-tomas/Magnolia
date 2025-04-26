#include "audio/audio_system.hpp"

#include "resources/audio.hpp"
#include "soloud/include/soloud.h"
#include "soloud/include/soloud_wav.h"

// @TODO: i like this type of system. Its simple, avoids pimping and its kinda nice to use when coupled with namespaces.
// Maybe ill refactor the other systems in the future to be more consistent and follow this approach.

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

        void play(ref<Audio>& audio, const f32 volume, const math::vec3& position, const math::vec3& velocity)
        {
            audio->handle = state->soloud.play3d(*static_cast<SoLoud::Wav*>(audio->source), position.x, position.y,
                                                 position.z, velocity.x, velocity.y, velocity.z, volume);
        }
    };  // namespace audio
};      // namespace mag
