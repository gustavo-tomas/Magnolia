#pragma once

#include "core/logger.hpp"
#include "core/types.hpp"

namespace mag
{
    class ECS;
    class ScriptingEngine
    {
        public:
            static void initialize();

            static void shutdown();

            // Warning! this unloads all registered scripts!
            static void new_state();

            static void load_script(const str& file_path);

            // @TODO: temp
            static void instanciate_script_for_entity(const ECS* ecs, const u32 entity_id);
            static void execute_create_method(const u32 entity_id);
            static void execute_destroy_method(const u32 entity_id);
            static void execute_update_method(const u32 entity_id, const f32 dt);
    };
};  // namespace mag
