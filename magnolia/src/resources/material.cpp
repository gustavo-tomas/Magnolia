#include "resources/material.hpp"

#include "core/application.hpp"

namespace mag
{
    std::shared_ptr<Material> MaterialManager::get(const str& name)
    {
        auto it = materials.find(name);
        if (it != materials.end())
        {
            return it->second;
        }

        auto& app = get_application();
        auto& job_system = app.get_job_system();
        auto& material_loader = app.get_material_loader();

        // Create a new material
        Material* material = new Material();
        material->name = "Placeholder";
        material->textures[TextureSlot::Albedo] = "magnolia/assets/images/DefaultAlbedoSeamless.png";
        material->textures[TextureSlot::Normal] = "magnolia/assets/images/DefaultNormal.png";

        materials[name] = std::shared_ptr<Material>(material);

        // Temporary material to load data into
        Material* transfer_material = new Material(*material);

        // Load in another thread
        auto execute = [&material_loader, name, transfer_material]
        {
            // If the load fails we still have valid data
            transfer_material->loading_state = MaterialLoadingState::LoadingInProgress;
            return material_loader.load(name, transfer_material);
        };

        // Callback when finished loading
        auto load_finished_callback = [material, transfer_material](const b8 result)
        {
            // Update the material and the renderer material data
            if (result == true)
            {
                transfer_material->loading_state = MaterialLoadingState::LoadingFinished;
                *material = *transfer_material;
            }

            // We can dispose of the temporary material now
            delete transfer_material;
        };

        Job load_job = Job(execute, load_finished_callback);
        job_system.add_job(load_job);

        return materials[name];
    }

    std::shared_ptr<Material> MaterialManager::get_default() { return get(DEFAULT_MATERIAL_NAME); }
};  // namespace mag
