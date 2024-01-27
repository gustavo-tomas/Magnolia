#include "renderer/sampler.hpp"

#include "renderer/context.hpp"

namespace mag
{
    void Sampler::initialize(const vk::Filter min_mag_filter, const vk::SamplerAddressMode address_mode,
                             const vk::SamplerMipmapMode mip_map_mode, const u32 mip_levels)
    {
        auto& context = get_context();

        vk::SamplerCreateInfo sampler_info({}, min_mag_filter, min_mag_filter, mip_map_mode, address_mode, address_mode,
                                           address_mode, 0.0f, true,
                                           context.get_physical_device().getProperties().limits.maxSamplerAnisotropy,
                                           false, {}, 0.0f, static_cast<f32>(mip_levels));

        this->sampler = context.get_device().createSampler(sampler_info);
    }

    void Sampler::shutdown()
    {
        auto& context = get_context();
        context.get_device().destroySampler(sampler);
    }
};  // namespace mag
