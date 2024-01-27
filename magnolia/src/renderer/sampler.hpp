#pragma once

#include "core/types.hpp"

namespace mag
{
    class Sampler
    {
        public:
            void initialize(const vk::Filter min_mag_filter, const vk::SamplerAddressMode address_mode,
                            const vk::SamplerMipmapMode mip_map_mode, const u32 mip_levels);
            void shutdown();

            const vk::Sampler& get_handle() const { return sampler; };

        private:
            vk::Sampler sampler;
    };
};  // namespace mag
