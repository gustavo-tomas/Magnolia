#pragma once

#include "core/types.hpp"

namespace mag
{
    enum class Filter
    {
        Nearest,
        Linear
    };

    enum class SamplerAddressMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
        MirrorClampToEdge
    };

    enum class SamplerMipmapMode
    {
        Nearest,
        Linear
    };

    class Sampler
    {
        public:
            Sampler(const Filter min_mag_filter, const SamplerAddressMode address_mode,
                    const SamplerMipmapMode mip_map_mode, const u32 mip_levels);
            ~Sampler();

            const void* get_handle() const;

        private:
            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
