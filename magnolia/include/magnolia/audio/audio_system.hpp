#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    struct AudioResource;

    namespace audio
    {
        b8 initialize();
        void shutdown();

        void register_audio_source(void* source);

        MAG_API void play(ref<AudioResource>& audio, f32 volume = 1.0F, const math::vec3& position = math::vec3(0.0F),
                          const math::vec3& velocity = math::vec3(0.0F));

        MAG_API void stop(const ref<AudioResource>& audio);
    };  // namespace audio
};  // namespace mag
