#include "renderer/sampler.hpp"

#include <vulkan/vulkan.hpp>

#include "private/renderer_type_conversions.hpp"
#include "renderer/context.hpp"

namespace mag
{
    struct Sampler::IMPL
    {
            vk::Sampler sampler;
    };

    Sampler::Sampler(const Filter min_mag_filter, const SamplerAddressMode address_mode,
                     const SamplerMipmapMode mip_map_mode, const u32 mip_levels)
        : impl(new IMPL())
    {
        auto& context = get_context();

        vk::SamplerCreateInfo sampler_info({}, mag_to_vk(min_mag_filter), mag_to_vk(min_mag_filter),
                                           mag_to_vk(mip_map_mode), mag_to_vk(address_mode), mag_to_vk(address_mode),
                                           mag_to_vk(address_mode), 0.0f, true,
                                           context.get_physical_device().getProperties().limits.maxSamplerAnisotropy,
                                           false, {}, 0.0f, static_cast<f32>(mip_levels));

        impl->sampler = context.get_device().createSampler(sampler_info);
    }

    Sampler::~Sampler()
    {
        auto& context = get_context();
        context.get_device().destroySampler(impl->sampler);
    }

    const void* Sampler::get_handle() const { return &impl->sampler; }
};  // namespace mag
