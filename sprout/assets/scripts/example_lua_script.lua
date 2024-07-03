-- For now, scripts only deal with logic, and cannot create new components/entities

function on_create(entity)
    print("Created!!!")
    print("ID: ", entity.entity_id)
end

function on_destroy(entity)
    print("Destroyed!!!")
end

function on_update(entity, dt)
    transform_component = entity:get_transform()
    camera_component = entity:get_camera()
    
    if (is_key_down(Keys.a)) then

        local pos = vec3.new(10, 20, 30) 

        transform_component.translation = pos
        camera_component.camera:set_position(transform_component.translation)
    end

    print("Camera:", camera_component.camera:get_position())

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
