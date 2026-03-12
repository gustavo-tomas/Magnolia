#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
    struct AudioResource : public IResource
    {
            u32 handle = 0;
            void* source = nullptr;
    };

    namespace resource
    {
        b8 load_sync(const str& file_path, ResourceManager* rm, AudioResource* resource);
    };  // namespace resource
};  // namespace mag
