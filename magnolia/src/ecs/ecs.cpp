#include "ecs/ecs.hpp"

#include "core/assert.hpp"
#include "ecs/components.hpp"

namespace mag
{
    using namespace mag::math;

    ECS::ECS(const u32 max_entity_id, ComponentAddedCallbackFn on_component_added)
        : on_component_added(on_component_added)
    {
        for (u32 id = 0; id <= max_entity_id; id++)
        {
            available_ids.insert(id);
        }
    }

    ECS::ECS(const ECS& other)
    {
        available_ids = other.available_ids;
        entities = copy_entities(other.entities);
        component_map = other.component_map;
        on_component_added = other.on_component_added;
    }

    ECS::~ECS() = default;

    // Return a new id (create a new entity)
    u32 ECS::create_entity(const str& name)
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

    void ECS::erase_entity(const u32 entity_id)
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

    // Get all ids in use
    std::vector<u32> ECS::get_entities_ids()
    {
        std::vector<u32> ids;
        for (auto& [id, entity] : entities)
        {
            ids.push_back(id);
        }

        return ids;
    }

    b8 ECS::entity_exists(const u32 id) const { return entities.contains(id); }

    std::map<u32, Entity> ECS::copy_entities(const std::map<u32, Entity>& source)
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
};  // namespace mag
