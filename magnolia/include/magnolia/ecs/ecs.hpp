#pragma once

#include <algorithm>
#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "magnolia/core/types.hpp"

namespace mag
{
    using EntityID = u32;

    using ComponentAddedCallbackFn = std::function<void(const EntityID entity_id, std::any component)>;

    // @TODO: We need this interface to properly store templated storages. Its not ideal since vector and other data
    // structures already allocate on the heap, so we would prefer to store them directly instead.
    class IComponentStorage
    {
        public:
            virtual ~IComponentStorage() = default;
            virtual void remove_component(EntityID entity_id) = 0;
            virtual b8 has_component(EntityID entity_id) const = 0;
    };

    template <typename Component>
    class ComponentStorage : public IComponentStorage
    {
        public:
            void add_component(const EntityID entity_id, Component&& component)
            {
                components.emplace(entity_id, std::move(component));
            }

            Component* get_component(const EntityID entity_id)
            {
                auto it = components.find(entity_id);
                return it != components.end() ? &it->second : nullptr;
            }

            void remove_component(const EntityID entity_id) override { components.erase(entity_id); }

            b8 has_component(const EntityID entity_id) const override
            {
                return components.find(entity_id) != components.end();
            }

        private:
            std::unordered_map<EntityID, Component> components;
    };

    class MAG_API ECS
    {
        public:
            ECS() = default;
            explicit ECS(ComponentAddedCallbackFn on_component_added)
                : on_component_added(std::move(on_component_added))
            {
            }
            ~ECS() = default;

            EntityID create_entity()
            {
                const EntityID entity_id = next_entity_id++;
                entities_ids.push_back(entity_id);

                return entity_id;
            }

            void erase_entity(const EntityID entity_id)
            {
                for (const auto& [type_id, storage] : component_storages)
                {
                    storage->remove_component(entity_id);
                }

                auto range = std::ranges::remove(entities_ids, entity_id);
                entities_ids.erase(range.begin(), range.end());
            }

            template <typename Component, typename... Args>
            void add_component(const EntityID entity_id, Args&&... args)
            {
                static_assert(std::is_constructible_v<Component, Args...>,
                              "Component must be constructible with provided arguments");

                ComponentStorage<Component>* storage = get_storage<Component>();
                Component component = Component(std::forward<Args>(args)...);
                storage->add_component(entity_id, std::move(component));

                if (on_component_added != nullptr)
                {
                    on_component_added(entity_id, *storage->get_component(entity_id));
                }
            }

            template <typename Component>
            Component* get_component(const EntityID entity_id)
            {
                ComponentStorage<Component>* storage = get_storage<Component>();
                return storage->get_component(entity_id);
            }

            template <typename... Components>
            std::tuple<Components*...> get_components(const EntityID entity_id)
            {
                std::tuple<Components*...> components;

                components = std::make_tuple(get_component<Components>(entity_id)...);

                return components;
            }

            template <typename Component>
            std::vector<Component*> get_all_components_of_type()
            {
                std::vector<Component*> components;

                for (const EntityID entity_id : entities_ids)
                {
                    if (Component* c = get_component<Component>(entity_id))
                    {
                        components.push_back(c);
                    }
                }

                return components;
            }

            template <typename... Components>
            std::vector<std::tuple<Components*...>> get_all_components_of_types()
            {
                const std::vector<EntityID> ids = query<Components...>();
                std::vector<std::tuple<Components*...>> components;

                for (const EntityID entity_id : ids)
                {
                    components.push_back(get_components<Components...>(entity_id));
                }

                return components;
            }

            template <typename... Components>
            std::vector<EntityID> query()
            {
                std::vector<EntityID> result;
                for (EntityID entity_id : entities_ids)
                {
                    if ((has_component<Components>(entity_id) && ...))
                    {
                        result.push_back(entity_id);
                    }
                }

                return result;
            }

            template <typename Component>
            b8 has_component(const EntityID entity_id) const
            {
                const std::type_index type_id = std::type_index(typeid(Component));
                auto it = component_storages.find(type_id);
                return it != component_storages.end() && it->second->has_component(entity_id);
            }

            template <typename Component>
            void remove_component(const EntityID entity_id)
            {
                ComponentStorage<Component>* storage = get_storage<Component>();
                storage->remove_component(entity_id);
            }

            b8 entity_exists(const EntityID entity_id) const
            {
                // @NOTE: Because a new entity id is always greater than the current id, the array is sorted and we can
                // use binary search. If this changes for whatever reason, the search also needs to be updated.
                return std::ranges::binary_search(entities_ids, entity_id);
            }

            const std::vector<EntityID>& get_entities_ids() const { return entities_ids; }

        private:
            template <typename Component>
            ComponentStorage<Component>* get_storage()
            {
                const std::type_index type_id = std::type_index(typeid(Component));
                auto it = component_storages.find(type_id);

                if (it == component_storages.end())
                {
                    unique<ComponentStorage<Component>> storage = create_unique<ComponentStorage<Component>>();
                    ComponentStorage<Component>* storage_ptr = storage.get();
                    component_storages[type_id] = std::move(storage);

                    return storage_ptr;
                }

                return static_cast<ComponentStorage<Component>*>(it->second.get());
            }

            EntityID next_entity_id = 0;
            std::vector<EntityID> entities_ids;
            std::unordered_map<std::type_index, unique<IComponentStorage>> component_storages;
            ComponentAddedCallbackFn on_component_added = nullptr;
    };
};  // namespace mag
