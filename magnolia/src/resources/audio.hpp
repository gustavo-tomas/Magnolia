#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Audio
    {
            u32 handle = 0;
            void* source = nullptr;
            str name;
    };

    namespace resource
    {
        ref<Audio> get_audio(const str& name);
    };
};  // namespace mag
