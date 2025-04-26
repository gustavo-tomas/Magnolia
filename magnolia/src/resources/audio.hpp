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

        private:
            std::map<str, ref<Audio>> audios;
    };
};  // namespace mag
