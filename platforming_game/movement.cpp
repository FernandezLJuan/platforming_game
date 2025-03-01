#include "movement.h"

Movement_System::Movement_System(entity_manager& em, Input_Handler& input, Collision_System& collision_system) : em(em), input(input), collision_sys(collision_system){}

Collision_System::Collision_System(entity_manager& em) : em(em) {}

void Movement_System::update(double delta_time) {
    const float MAX_JUMP_HOLD_TIME = 3.0f;
    const float MIN_JUMP_STRENGTH = -20.0f;

    // Delayed jump flag
    static bool delayed_jump = false;
    static unsigned int delayed_jump_key = 0;

    for (auto& e : em.entities) {
        if (e.mask.test(components::get_id<components::movement>())) {
            auto* position = em.get_component<components::position>(e.id);
            auto* movement = em.get_component<components::movement>(e.id);

            // Apply gravity
            if (e.mask.test(components::get_id<components::gravity>())) {
                auto* gravity_component = em.get_component<components::gravity>(e.id);

                if (!gravity_component->is_grounded) {
                    movement->speed.y += gravity_component->falling_strength * delta_time;
                }
            }

            //handle input (after collision detection)
            if (e.mask.test(components::get_id<components::input>())) {
                auto* key_binds = em.get_component<components::input>(e.id);

                if (input.is_key_pressed(key_binds->move_left)) {
                    double key_press_duration = input.key_time(SDL_SCANCODE_A);
                    movement->speed.x -= movement->acceleration.x * key_press_duration * delta_time;
                }
                else if (movement->speed.x < 0) {
                    movement->speed.x += movement->deceleration.x * delta_time;
                }

                if (input.is_key_pressed(key_binds->move_right)) {
                    double key_press_duration = input.key_time(SDL_SCANCODE_D);
                    movement->speed.x += movement->acceleration.x * key_press_duration * delta_time;
                }
                else if (movement->speed.x > 0) {
                    movement->speed.x -= movement->deceleration.x * delta_time;
                }

                if (std::abs(movement->speed.x) > movement->max_speed.x) {
                    float sign = std::signbit(movement->speed.x) ? -1.0f : 1.0f;
                    movement->speed.x = movement->max_speed.x * sign;
                }

                if (e.mask.test(components::get_id<components::gravity>())) {
                    auto* gravity_component = em.get_component<components::gravity>(e.id);

                    if (input.is_key_pressed(key_binds->jump) && gravity_component->is_grounded) {
                        movement->speed.y = gravity_component->jump_strength;
                    }

                    if (input.is_key_released(key_binds->jump) && !gravity_component->is_grounded) {
                        movement->speed.y *= 0.5;
                    }
                }
            }

            //calculate new position
            float new_x = position->pos.x + movement->speed.x * delta_time;
            float new_y = position->pos.y + movement->speed.y * delta_time;

            //check for collisions with other entities
            if (e.mask.test(components::get_id<components::collision>())) {
                auto* collision_component = em.get_component<components::collision>(e.id);

                //reset is_grounded at the start of the collision check
                if (e.mask.test(components::get_id<components::gravity>())) {
                    auto* gravity_component = em.get_component<components::gravity>(e.id);
                    gravity_component->is_grounded = false; // Assume not grounded unless a bottom collision is detected
                }

                //temporarily update the hitbox to the new position
                collision_component->hitbox.x = new_x;
                collision_component->hitbox.y = new_y;

                //check for collisions with all other entities
                for (auto& other_entity : em.entities) {
                    if (&e != &other_entity && other_entity.mask.test(components::get_id<components::collision>())) {
                        auto collision_type = collision_sys.detect_collision(e, other_entity);

                        if (collision_type != Collision_System::collision_direction::NO_COLLISION) {
                            // Handle collision
                            switch (collision_type) {
                            case Collision_System::collision_direction::TOP_COLLISION:
                                new_y = em.get_component<components::collision>(other_entity.id)->hitbox.y - collision_component->hitbox.h;
                                movement->speed.y = 0.0f; // Stop vertical movement
                                break;

                            case Collision_System::collision_direction::BOTTOM_COLLISION:
                                new_y = em.get_component<components::collision>(other_entity.id)->hitbox.y - collision_component->hitbox.h;
                                movement->speed.y = 0.0f; // Stop vertical movement

                                // Set grounded flag if the entity has a gravity component
                                if (e.mask.test(components::get_id<components::gravity>())) {
                                    auto* gravity_component = em.get_component<components::gravity>(e.id);
                                    gravity_component->is_grounded = true;

                                }
                                break;

                            case Collision_System::collision_direction::LEFT_COLLISION:
                                new_x = em.get_component<components::collision>(other_entity.id)->hitbox.x - collision_component->hitbox.w;
                                movement->speed.x = 0.0f; // Stop horizontal movement
                                break;

                            case Collision_System::collision_direction::RIGHT_COLLISION:
                                new_x = em.get_component<components::collision>(other_entity.id)->hitbox.x + em.get_component<components::collision>(other_entity.id)->hitbox.w;
                                movement->speed.x = 0.0f; // Stop horizontal movement
                                break;
                            }
                        }
                    }
                }

                // Update the hitbox to the final position
                collision_component->hitbox.x = new_x;
                collision_component->hitbox.y = new_y;
            }

            // Apply the final position
            position->pos.x = new_x;
            position->pos.y = new_y;

            
        }
    }
}

Collision_System::collision_direction Collision_System::detect_collision(entity& e1, entity& e2) {
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
        return collision_direction::TOP_COLLISION;
    }
    else if (minOverlap == overlapBottom) {
        return collision_direction::BOTTOM_COLLISION;
    }
    else if (minOverlap == overlapLeft) {
        return collision_direction::LEFT_COLLISION;
    }
    else if (minOverlap == overlapRight) {
        return collision_direction::RIGHT_COLLISION;
    }

    return collision_direction::NO_COLLISION;
}