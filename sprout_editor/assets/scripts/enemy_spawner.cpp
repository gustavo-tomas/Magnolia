#include <ecs/components.hpp>
#include <magnolia.hpp>
#include <math/generic.hpp>
#include <resources/model.hpp>

using namespace mag;

class EnemySpawner : public ScriptableEntity
{
    public:
        virtual void on_create() override { LOG_SUCCESS("Created EnemySpawner"); }

        virtual void on_destroy() override { LOG_SUCCESS("Destroyed EnemySpawner"); }

        // Spawn some enemies
        virtual void on_update(const f32 dt) override
        {
            static f32 timer = 0;

            timer += dt;

            auto [transform] = get_components<TransformComponent>();
            if (!transform)
            {
                LOG_WARNING("(EnemySpawner) Missing transform");
                return;
            }

            if (timer >= 1.0f)
            {
                timer = 0;

                spawn_enemy();
            }
        }

        void spawn_enemy()
        {
            static u32 counter = 0;

            LOG_INFO("Enemies spawned: {0}", counter++);

            Application& app = get_application();
            ModelManager& model_manager = app.get_model_manager();

            const TransformComponent* spawner_transform = get_component<TransformComponent>();

            if (!spawner_transform)
            {
                LOG_WARNING("(EnemySpawner) Missing transform");
                return;
            }

            // Spawn an enemy
            const u32 enemy_id = create_entity("MenacingHammer" + std::to_string(counter));

            const ref<Model> model =
                model_manager.get("sprout_editor/assets/models/hammer/native/wooden_hammer_01.model.json");

            TransformComponent* enemy_transform = new TransformComponent(*spawner_transform);
            enemy_transform->scale = vec3(100.0f);

            add_component_to_entity(enemy_id, enemy_transform);
            add_component_to_entity(enemy_id, new ModelComponent(model));
            add_component_to_entity(enemy_id, new ScriptComponent("sprout_editor/assets/scripts/enemy_controller.cpp"));
        }

        virtual void on_event(const Event& e) override { (void)e; }
};

extern "C" ScriptableEntity* create_script() { return new EnemySpawner(); }
extern "C" void destroy_script(ScriptableEntity* script) { delete script; }
