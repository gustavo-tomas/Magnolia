// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "resources/audio.hpp"
#include "soloud/include/soloud_wav.h"

namespace mag
{
    namespace resource
    {
        b8 load(const str &file_path, Audio *audio)
        {
            if (!audio)
            {
                LOG_ERROR("Invalid audio ptr");
                return false;
            }

            Buffer buffer;
            if (!fs::read_binary_data(file_path, buffer))
            {
                LOG_ERROR("Failed to load audio file: {0}", file_path);
                return false;
            }

            SoLoud::Wav *audio_source = new SoLoud::Wav();

            const SoLoud::result result = audio_source->loadMem(buffer.data.data(), buffer.get_size(), true);
            if (result != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR)
            {
                LOG_ERROR("Failed to load audio: '{0}'", file_path);
                return false;
            }

            // Update audio data
            audio->source = audio_source;

            return true;
        }
    };  // namespace resource
};      // namespace mag
