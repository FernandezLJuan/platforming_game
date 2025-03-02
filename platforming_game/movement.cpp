#include "movement.h"

Movement_System::Movement_System(entity_manager& em, Input_Handler& input, Collision_System& collision_system) : em(em), input(input){}

Collision_System::Collision_System(entity_manager& em) : em(em) {}

void Movement_System::update(double delta_time) {

    for (auto& e : em.entities) {
        if (e.mask.test(components::get_id<components::movement>())) {
            auto* position = em.get_component<components::position>(e.id);
            auto* movement = em.get_component<components::movement>(e.id);

            // Apply gravity
            if (e.mask.test(components::get_id<components::gravity>())) {
                auto* gravity_component = em.get_component<components::gravity>(e.id);
                movement->speed.y += gravity_component->falling_strength * delta_time;
            }

            // Handle input (after collision detection)
            if (e.mask.test(components::get_id<components::input>())) {
                auto* key_binds = em.get_component<components::input>(e.id);

                if (input.is_key_pressed(key_binds->move_left)) {
                    double key_press_duration = input.key_time(SDL_SCANCODE_A);
                    movement->speed.x -= movement->acceleration.x * key_press_duration * delta_time;
                }

                if (input.is_key_pressed(key_binds->move_right)) {
                    double key_press_duration = input.key_time(SDL_SCANCODE_D);
                    movement->speed.x += movement->acceleration.x * key_press_duration * delta_time;
                }

                if (!input.is_key_pressed(key_binds->move_left) && !input.is_key_pressed(key_binds->move_right)) {
                    if (std::abs(movement->speed.x) < 0.01) {
                        movement->speed.x = 0;  // Detener completamente si es muy baja
                    }
                    else if (movement->speed.x > 0) {
                        movement->speed.x -= movement->deceleration.x * delta_time;
                        if (movement->speed.x < 0) movement->speed.x = 0;  // Evita moverse en reversa
                    }
                    else if (movement->speed.x < 0) {
                        movement->speed.x += movement->deceleration.x * delta_time;
                        if (movement->speed.x > 0) movement->speed.x = 0;  // Evita moverse en reversa
                    }
                }


                if (std::abs(movement->speed.x) > movement->max_speed.x) {
                    float sign = std::signbit(movement->speed.x) ? -1.0f : 1.0f;
                    movement->speed.x = movement->max_speed.x * sign;
                }

                if (e.mask.test(components::get_id<components::jump>())) {
                    auto* jump_component = em.get_component<components::jump>(e.id);

                    if (input.is_key_pressed(key_binds->jump) && position->is_grounded) {
                        movement->speed.y = jump_component->jump_strength;
                        position->is_grounded = false;
                    }

                    if (input.is_key_released(key_binds->jump) && !position->is_grounded) {
                        movement->speed.y *= 0.5;
                    }
                }
            }

            if (std::abs(movement->speed.y) > movement->max_speed.y) {
                float sign = std::signbit(movement->speed.y) ? -1.0f : 1.0f;
                movement->speed.y = movement->max_speed.x * sign;
            }

            //apply the final position
            position->pos.x += movement->speed.x;
            position->pos.y += movement->speed.y;

            if (e.mask.test(components::get_id<components::collision>())) {
                auto* collision_component = em.get_component<components::collision>(e.id);

                collision_component->hitbox.x = position->pos.x;
                collision_component->hitbox.y = position->pos.y;
            }
        }
    }
}

Collision_System::collision_direction Collision_System::detect_collision(entity& e1, entity& e2) {

    collision_direction direction = collision_direction::NO_COLLISION;

    auto* collision_component_1 = em.get_component<components::collision>(e1.id);
    auto* collision_component_2 = em.get_component<components::collision>(e2.id);

    if (!collision_component_1 || !collision_component_2) {
        return collision_direction::NO_COLLISION;
    }

    // Use the hitboxes directly (assuming they are in world coordinates)
    const SDL_FRect& hitbox_1 = collision_component_1->hitbox;
    const SDL_FRect& hitbox_2 = collision_component_2->hitbox;

    // Check for general collision
    if (hitbox_1.x + hitbox_1.w <= hitbox_2.x || hitbox_2.x + hitbox_2.w <= hitbox_1.x ||
        hitbox_1.y + hitbox_1.h <= hitbox_2.y || hitbox_2.y + hitbox_2.h <= hitbox_1.y) {
        return collision_direction::NO_COLLISION;
    }

    // Calculate overlap amounts
    float overlapLeft = hitbox_1.x + hitbox_1.w - hitbox_2.x;
    float overlapRight = hitbox_2.x + hitbox_2.w - hitbox_1.x;
    float overlapBottom = hitbox_1.y + hitbox_1.h - hitbox_2.y;
    float overlapTop = hitbox_2.y + hitbox_2.h - hitbox_1.y;

    // Default to the smallest overlap if no movement direction is dominant
    float minOverlap = std::min({ overlapLeft, overlapRight, overlapTop, overlapBottom });

    if (minOverlap == overlapTop) {
        direction = collision_direction::TOP_COLLISION;
    }
    else if (minOverlap == overlapBottom) {
        direction = collision_direction::BOTTOM_COLLISION;
    }
    else if (minOverlap == overlapLeft) {
        direction = collision_direction::LEFT_COLLISION;
    }
    else if (minOverlap == overlapRight) {
        direction = collision_direction::RIGHT_COLLISION;
    }

    resolve_collision(e1, e2, direction);

    return direction;
}

void Collision_System::resolve_collision(entity& e1, entity& e2, collision_direction direction) {

    auto* e1_collision = em.get_component<components::collision>(e1.id);
    auto* e2_collision = em.get_component<components::collision>(e2.id);

    //resolve rigid body collisions
    if (e1_collision->is_rigid || e2_collision->is_rigid) {
        resolve_rigid_collision(e1, e2, direction);
    }

    //resolve health-damage interactions
    resolve_health_damage(e1, e2);
}

void Collision_System::resolve_rigid_collision(entity& e1, entity& e2, collision_direction direction) {
    auto* e1_movement = em.get_component<components::movement>(e1.id);
    auto* e2_movement = em.get_component<components::movement>(e2.id);

    // Ensure neither entity is moved if it does not have a movement component
    if (!e1_movement && !e2_movement) return;

    auto* e1_position = em.get_component<components::position>(e1.id);
    auto* e2_position = em.get_component<components::position>(e2.id);

    auto& e1_hitbox = em.get_component<components::collision>(e1.id)->hitbox;
    auto& e2_hitbox = em.get_component<components::collision>(e2.id)->hitbox;

    float x = 0;

    switch (direction) {
    case collision_direction::BOTTOM_COLLISION:
        if (e1_position) e1_position->is_grounded = true;
        if (e1_movement) e1_movement->speed.y = 0.0;

        x = (e1_hitbox.y + e1_hitbox.h) - e2_hitbox.y;

        // Move only if the entity has movement component
        if (e1_movement) e1_position->pos.y -= x;
        if (e2_movement && e2_position) e2_position->pos.y += x / 2.0f;
        break;

    case collision_direction::TOP_COLLISION:
        if (e1_movement) e1_movement->speed.y = 0.0;

        x = (e2_hitbox.y + e2_hitbox.h) - e1_hitbox.y;

        if (e1_movement) e1_position->pos.y += x;
        if (e2_movement && e2_position) e2_position->pos.y -= x / 2.0f;
        break;

    case collision_direction::LEFT_COLLISION:
        if (e1_movement) e1_movement->speed.x = 0.0;

        x = (e1_hitbox.x + e1_hitbox.w) - e2_hitbox.x;

        if (e1_movement) e1_position->pos.x -= x;
        if (e2_movement && e2_position) e2_position->pos.x += x / 2.0f;
        break;

    case collision_direction::RIGHT_COLLISION:
        if (e1_movement) e1_movement->speed.x = 0.0;

        x = (e2_hitbox.x + e2_hitbox.w) - e1_hitbox.x;

        if (e1_movement) e1_position->pos.x += x;
        if (e2_movement && e2_position) e2_position->pos.x -= x / 2.0f;
        break;
    }

    // Update hitboxes only for entities that actually moved
    if (e1_position) {
        e1_hitbox.x = e1_position->pos.x;
        e1_hitbox.y = e1_position->pos.y;
    }
    if (e2_position && e2_movement) {
        e2_hitbox.x = e2_position->pos.x;
        e2_hitbox.y = e2_position->pos.y;
    }
}

void Collision_System::resolve_health_damage(entity& e1, entity& e2) {

    if (e1.mask.test(components::get_id<components::health>()) &&
        e2.mask.test(components::get_id<components::damage>())) {
        auto* e1_health = em.get_component<components::health>(e1.id);
        auto* e2_damage = em.get_component<components::damage>(e2.id);

        em.assign_component<components::pending_damage>(e1.id);
        auto* pending_damage = em.get_component<components::pending_damage>(e1.id);

        pending_damage->pending_amount += e2_damage->damage_amount;
    }
}