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
        Error
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
