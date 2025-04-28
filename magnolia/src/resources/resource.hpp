#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    enum class LoadingStatus
    {
        Pending,
        InProgress,
        Finished,
        Error,

        // @TODO: this is kind of a hack that we need to keep track of
        // because some of our resource subsystems are coupled to the renderer
        UploadedToGpu
    };

    // Interface for a resource
    struct IResource
    {
            LoadingStatus loading_status = LoadingStatus::Pending;
    };

    namespace resource
    {
        // Initialize all resource subsystems
        b8 initialize();

        // Shutdown all resource subsystems
        void shutdown();
    };  // namespace resource
};      // namespace mag
