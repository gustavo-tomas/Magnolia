#pragma once

#include <functional>

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
            virtual ~IResource() = default;
            LoadingStatus loading_status = LoadingStatus::Pending;
    };

    typedef std::function<void(const IResource*)> ResourceLoadedCallbackFn;

    namespace resource
    {
        // Initialize all resource subsystems
        b8 initialize();

        // Shutdown all resource subsystems
        void shutdown();

        // Set a callback to be called whenever a resource finishes loading
        void set_on_resource_loaded_callback(const ResourceLoadedCallbackFn& callback);
    };  // namespace resource
};      // namespace mag
