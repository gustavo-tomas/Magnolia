#include "resources/audio.hpp"

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "soloud/include/soloud_wav.h"

namespace mag
{
    namespace resource
    {
        AudioLoader::AudioLoader() {}

        AudioLoader::~AudioLoader() {}

        IResource *AudioLoader::load(const str &file_path)
        {
            AudioResource *audio = new AudioResource();

            if (!audio)
            {
                LOG_ERROR("Invalid audio ptr");
                delete audio;
                return nullptr;
            }

            Buffer buffer;
            if (!fs::read_binary_data(file_path, buffer))
            {
                LOG_ERROR("Failed to load audio file: {0}", file_path);
                delete audio;
                return nullptr;
            }

            SoLoud::Wav *audio_source = new SoLoud::Wav();

            const SoLoud::result result = audio_source->loadMem(buffer.data.data(), buffer.get_size(), true);
            if (result != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR)
            {
                LOG_ERROR("Failed to load audio: '{0}'", file_path);
                delete audio;
                return nullptr;
            }

            // Update audio data
            audio->name = file_path;
            audio->file_path = file_path;
            audio->source = audio_source;

            return audio;
        }
    };  // namespace resource
};      // namespace mag
