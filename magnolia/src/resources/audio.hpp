#pragma once

#include "core/types.hpp"
#include "resources/resource.hpp"

namespace mag
{
    struct Audio : public IResource
    {
            u32 handle = 0;
            void* source = nullptr;
            str name;
    };

    namespace resource
    {
        ref<Audio> get_audio(const str& name);
    };  // namespace resource
};      // namespace mag
