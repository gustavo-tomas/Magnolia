#pragma once

#include <map>

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    struct Audio
    {
            u32 handle = 0;
            void* source = nullptr;
            str name;
    };

    class AudioManager
    {
        public:
            AudioManager();
            ~AudioManager();

            ref<Audio> get(const str& name);

            void play(ref<Audio>& audio, const f32 volume = 1.0f, const math::vec3& position = math::vec3(0.0f),
                      const math::vec3& velocity = math::vec3(0.0f));

        private:
            std::map<str, ref<Audio>> audios;
    };
};  // namespace mag
