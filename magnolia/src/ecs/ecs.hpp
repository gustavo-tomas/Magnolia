#pragma once

#include <map>
#include <memory>
#include <set>
#include <typeindex>
#include <vector>

#include "core/logger.hpp"
#include "core/types.hpp"
#include "ecs/components.hpp"

namespace mag
{
    using namespace mag::math;

    // @TODO: errors are just logs but might be better if they are assertions
    class ECS
    {
        public:
            using Entity = std::vector<std::unique_ptr<Component>>;

            ECS(const u32 max_entity_id = 10'000)
            {
                for (u32 id = 0; id <= max_entity_id; id++) available_ids.insert(id);
            }

            ~ECS() = default;

            // Return a new id (create a new entity)
            u32 create_entity()
            {
                ASSERT(!available_ids.empty(), "No available IDs left");

                const u32 id = *available_ids.begin();
                available_ids.erase(id);

                entities[id] = Entity();

                return id;
            }

            // Add a component to the entity
            template <typename T>
            void add_component(const u32 entity_id, T* c)
            {
                static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

                // Check if entity exists
                if (!entities.contains(entity_id))
                {
                    LOG_ERROR("Entity with ID: {0} does not exist", entity_id);
                    return;
                }

                // Check if component already exists
                if (get_component<T>(entity_id) != nullptr)
                {
                    LOG_ERROR("Entity with ID: {0} already has that component", entity_id);
                    return;
                }

                entities[entity_id].emplace_back(c);
                component_map[typeid(T)].insert(entity_id);
            }

            // Get component of that type
            template <typename T>
            T* get_component(const u32 entity_id)
            {
                static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

                // Validate ID
                auto entity = entities.find(entity_id);
                if (entity == entities.end())
                {
                    LOG_ERROR("No entity with ID: {0}", entity_id);
                    return nullptr;
                }

                // Search for the component
                for (auto& c : entity->second)
                {
                    if (auto derived = dynamic_cast<T*>(c.get()))
                    {
                        return derived;
                    }
                }

                // Component not found
                return nullptr;
            }

            // Get all components of that type
            template <typename T>
            std::vector<T*> get_components()
            {
                static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

                std::vector<T*> components;

                for (auto& id : component_map[typeid(T)])
                {
                    if (auto c = get_component<T>(id))
                    {
                        components.push_back(c);
                    }
                }

                return components;
            }

        private:
            // Entities IDs
            std::set<u32> available_ids;

            // Table of entities
            std::map<u32, Entity> entities;

            // Map from component type to entities that have it
            std::map<std::type_index, std::set<u32>> component_map;
    };
};  // namespace mag