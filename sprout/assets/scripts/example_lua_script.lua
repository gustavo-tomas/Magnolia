-- For now, scripts only deal with logic, and cannot create new components/entities

function on_create(entity)
    print("Created!!!")
    print("ID: ", entity.entity_id)
end

function on_destroy(entity)
    print("Destroyed!!!")
end

function on_update(entity, dt)
    print("DT: ", dt)

    local transform_component = entity:get_transform()
    -- local camera_component = entity:get_camera()

    print("Position:", transform_component.translation)

    -- @TODO: finish this script to be equal to the native example

    -- vec3 direction(0.0f);
    -- const f32 speed = 50.0f;

    -- if (is_key_down(Keys.a)) direction.x -= 1.0f;
    -- if (is_key_down(Keys.d)) direction.x += 1.0f;
    -- if (is_key_down(Keys.w)) direction.z -= 1.0f;
    -- if (is_key_down(Keys.s)) direction.z += 1.0f;
    -- if (is_key_down(Keys.Space)) direction.y += 1.0f;
    -- if (is_key_down(Keys.Lctrl)) direction.y -= 1.0f;

    -- -- Prevent nan values
    -- if (length(direction) > 0.0f) direction = normalize(direction) * dt;

    -- transform->translation += direction * speed;
    -- camera->set_position(transform->translation);

    -- print("POS:", math::to_string(camera->get_position()))
end
