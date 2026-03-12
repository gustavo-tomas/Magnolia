#include "magnolia/resources/audio.hpp"

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"
#include "soloud/include/soloud_wav.h"

namespace mag
{
    namespace resource
    {
        b8 load_sync(const str& file_path, ResourceManager* rm, AudioResource* resource)
        {
            (void)rm;

            Buffer buffer;
            if (!fs::read_binary_data(file_path, buffer))
            {
                LOG_ERROR("Failed to load audio file: {0}", file_path);
                return false;
            }

            auto* audio_source = new SoLoud::Wav();

            const SoLoud::result result = audio_source->loadMem(buffer.data.data(), buffer.get_size(), true);
            if (result != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR)
            {
                LOG_ERROR("Failed to load audio: '{0}'", file_path);
                return false;
            }

            // Update audio data
            resource->name = file_path;
            resource->file_path = file_path;
            resource->source = audio_source;

            return true;
        }
    };  // namespace resource
};  // namespace mag
