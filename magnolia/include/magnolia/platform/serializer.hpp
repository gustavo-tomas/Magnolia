#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>

#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"

namespace mag
{
    struct Buffer;

    namespace fs
    {
        // Data serializer
        class Serializer
        {
            public:
                // Register save handler for a specific type
                template <typename T>
                void register_on_save_handler(std::function<void(T&, Serializer&)> handler)
                {
                    save_handlers[std::type_index(typeid(T))] = [handler](void* data, Serializer& s)
                    { handler(*static_cast<T*>(data), s); };
                }

                // Register load handler for a specific type
                template <typename T>
                void register_on_load_handler(std::function<void(T&, Serializer&)> handler)
                {
                    load_handlers[std::type_index(typeid(T))] = [handler](void* data, Serializer& s)
                    { handler(*static_cast<T*>(data), s); };
                }

                // Save an object using registered handler
                template <typename T>
                b8 save(const str& file_path, T& obj)
                {
                    // Clear state
                    json.clear();

                    auto it = save_handlers.find(std::type_index(typeid(T)));
                    if (it != save_handlers.end())
                    {
                        it->second(&obj, *this);

                        return mag::fs::write_json_data(file_path, json);
                    }

                    LOG_ERROR("No save handler registered for this type");
                    return false;
                }

                // Load an object using registered handler
                template <typename T>
                b8 load(const str& file_path, T& obj)
                {
                    // Clear state
                    json.clear();

                    auto it = load_handlers.find(std::type_index(typeid(T)));
                    if (it != load_handlers.end())
                    {
                        const b8 result = mag::fs::read_json_data(file_path, json);

                        if (result)
                        {
                            it->second(&obj, *this);
                        }

                        return result;
                    }

                    LOG_ERROR("No load handler registered for this type");
                    return false;
                }

                // We can also add a buffer to support other data types if necessary
                mag::fs::json json;

            private:
                std::unordered_map<std::type_index, std::function<void(void*, Serializer&)>> save_handlers;
                std::unordered_map<std::type_index, std::function<void(void*, Serializer&)>> load_handlers;
        };
    };  // namespace fs
};  // namespace mag
