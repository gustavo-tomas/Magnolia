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
        class AudioLoader : public IResourceLoader
        {
            public:
                AudioLoader();
                ~AudioLoader() override;

                IResource* load_sync(const str& file_path) override;
        };
    };  // namespace resource
};  // namespace mag
