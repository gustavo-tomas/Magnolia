#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"
#include "magnolia/platform/window.hpp"

namespace mag
{
    struct MAG_API EngineInitializeOptions
    {
            window::WindowOptions window_options = {};
            gfx::GfxOptions gfx_options = {};
    };

    MAG_API b8 initialize(const EngineInitializeOptions& options);
    MAG_API void shutdown();

    MAG_API EngineInitializeOptions read_config_file(const str& file_path);
};  // namespace mag
