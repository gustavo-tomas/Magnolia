#pragma once

#include <map>
#include <memory>
#include <set>
#include <typeindex>
#include <vector>

#include "core/assert.hpp"
#include "core/logger.hpp"
#include "core/types.hpp"
#include "ecs/components.hpp"

namespace mag
{
    using namespace mag::math;

    typedef std::function<void(const u32 id, Component* component)> ComponentAddedCallbackFn;

    class ECS
    {
#define ASSERT_TYPE(T) static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component")

#define ASSERT_TYPES(Ts) \
    static_assert((std::is_base_of<Component, Ts>::value && ...), "All types must be derived from Component")

        public:
            using Entity = std::vector<unique<Component>>;

            ECS(const u32 max_entity_id = 10'000, ComponentAddedCallbackFn on_component_added = nullptr)
                : on_component_added(on_component_added)
            {
                for (u32 id = 0; id <= max_entity_id; id++)
                {
                    available_ids.insert(id);
                }
            }

            ECS(const ECS& other)
            {
                available_ids = other.available_ids;
                entities = copy_entities(other.entities);
                component_map = other.component_map;
                on_component_added = other.on_component_added;
            }

            ~ECS() = default;

            // Return a new id (create a new entity)
            u32 create_entity(const str& name = {})
            {
                ASSERT(!available_ids.empty(), "No available IDs left");

                const u32 id = *available_ids.begin();
                available_ids.erase(id);

                entities[id] = Entity();

                // Set a name
                str entity_name = name;
                if (entity_name.empty())
                {
                    entity_name = "Entity" + std::to_string(id);
                }

                add_component(id, new NameComponent(entity_name));

                return id;
            }

            void erase_entity(const u32 entity_id)
            {
                // Check if entity exists
                if (!entities.contains(entity_id))
                {
                    LOG_ERROR("Entity with ID: {0} does not exist", entity_id);
                    return;
                }

                // Erase the entity
                entities.erase(entity_id);

                // Remove the entity from the component map
                for (auto& [component_type, entity_set] : component_map)
                {
                    entity_set.erase(entity_id);
                }

                // Free the ID for future use
                available_ids.insert(entity_id);
            }

            // Add a component to the entity
            template <typename T>
            void add_component(const u32 entity_id, T* c)
            {
                ASSERT_TYPE(T);

                if (!entity_exists(entity_id))
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

                if (on_component_added)
                {
                    on_component_added(entity_id, c);
                }
            }

            // Get component of that type
            template <typename T>
            T* get_component(const u32 entity_id)
            {
                ASSERT_TYPE(T);

                if (!entity_exists(entity_id)) return nullptr;

                auto& entity = entities[entity_id];

                // Search for the component
                for (auto& c : entity)
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
            std::vector<T*> get_all_components_of_type()
            {
                ASSERT_TYPE(T);

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

            // Get entities with specific components
            template <typename... Ts>
            std::vector<u32> get_entities_with_components_of_type()
            {
                ASSERT_TYPES(Ts);

                std::vector<u32> ids;
                std::vector<std::type_index> component_types = {typeid(Ts)...};

                // Iterate over all entities
                for (auto& [id, entity] : entities)
                {
                    b8 has_all_components = true;

                    // Check if entity has all components
                    for (auto& component_type : component_types)
                    {
                        if (component_map[component_type].find(id) == component_map[component_type].end())
                        {
                            has_all_components = false;
                            break;
                        }
                    }

                    if (has_all_components)
                    {
                        ids.push_back(id);
                    }
                }

                return ids;
            }

            // Get the specified components of that entity
            template <typename... Ts>
            std::tuple<Ts*...> get_components(const u32 id)
            {
                ASSERT_TYPES(Ts);

                std::tuple<Ts*...> components;

                components = std::make_tuple(get_component<Ts>(id)...);

                return components;
            }

            // Get components of entities with the specified components
            template <typename... Ts>
            std::vector<std::tuple<Ts*...>> get_all_components_of_types()
            {
                ASSERT_TYPES(Ts);

                std::vector<u32> entity_ids = get_entities_with_components_of_type<Ts...>();
                std::vector<std::tuple<Ts*...>> components;

                for (auto& id : entity_ids)
                {
                    components.push_back(get_components<Ts...>(id));
                }

                return components;
            }

            // Get all ids in use
            std::vector<u32> get_entities_ids()
            {
                std::vector<u32> ids;
                for (auto& [id, entity] : entities)
                {
                    ids.push_back(id);
                }

                return ids;
            }

            b8 entity_exists(const u32 id) const { return entities.contains(id); }

        private:
            std::map<u32, Entity> copy_entities(const std::map<u32, Entity>& source)
            {
                std::map<u32, Entity> new_entities;
                for (const auto& [id, entity] : source)
                {
                    Entity new_entity;
                    for (const auto& comp : entity)
                    {
                        new_entity.emplace_back(comp->clone());
                    }
                    new_entities[id] = std::move(new_entity);
                }

                return new_entities;
            }

            // Entities IDs
            std::set<u32> available_ids;

            // Table of entities
            std::map<u32, Entity> entities;

            // Map from component type to entities that have it
            std::map<std::type_index, std::set<u32>> component_map;

            // Callback to signal when a component is added to an entity
            ComponentAddedCallbackFn on_component_added;
    };
};  // namespace mag
