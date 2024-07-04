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

    local direction = vec3(0)
    local speed = 50.0

    if (is_key_down(Keys.a)) then direction.x = direction.x - 1.0 end
    if (is_key_down(Keys.d)) then direction.x = direction.x + 1.0 end
    if (is_key_down(Keys.w)) then direction.z = direction.z - 1.0 end
    if (is_key_down(Keys.s)) then direction.z = direction.z + 1.0 end
    if (is_key_down(Keys.Space)) then direction.y = direction.y + 1.0 end
    if (is_key_down(Keys.Lctrl)) then direction.y = direction.y - 1.0 end

    -- Prevent nan values
    if (direction:length() > 0) then
        direction = direction:normalize() * dt
    end

    transform_component.translation = transform_component.translation + direction * speed
    camera_component.camera:set_position(transform_component.translation)

    print("POS:", camera_component.camera:get_position())
end
