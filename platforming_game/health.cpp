#include "health_system.h"

Health_System::Health_System(entity_manager& em) : em(em) {}

void Health_System::update(double delta_time) {
	for (auto& e : em.entities) {
		if (e.mask.test(components::get_id<components::health>())) {

			if (e.mask.test(components::get_id<components::invincibility>())) {
				auto* invincibility = em.get_component<components::invincibility>(e.id);
				std::cout << "Invincibility remaining time: " << invincibility->remaining_time << "\n";
				if (invincibility->remaining_time > 0.0) {
					invincibility->remaining_time -= delta_time;
				}
				else {
					// Remove the invincibility component when I-frames expire
					em.remove_component<components::invincibility>(e.id);
				}
			}

			if (e.mask.test(components::get_id<components::pending_damage>())) {
				damage_entity(e.id, em.get_component<components::pending_damage>(e.id)->pending_amount);
				em.remove_component<components::pending_damage>(e.id);
			}

			//apply regeneration effect
			if (e.mask.test(components::get_id<components::regeneration>())) {
				heal_entity(e.id, em.get_component<components::regeneration>(e.id)->regen_amount);
			}

			//apply thorns effect
			if (e.mask.test(components::get_id<components::thorns>())) {
				damage_entity(e.id, em.get_component<components::thorns>(e.id)->damage);
			}
		}
	}
}

void Health_System::damage_entity(unsigned long long id, int amount) {

	if (em.entities[id].mask.test(components::get_id<components::invincibility>())) {
		return;
	}

	auto* entity_health = em.get_component<components::health>(id);

	entity_health->current_health -= amount;

	if (entity_health->current_health <= 0) {
		em.delete_entity(id);
	}

	if (entity_health->current_health > 0) {
		activate_iframes(id, entity_health->i_frames);
	}
}

void Health_System::heal_entity(unsigned long long id, int amount) {
	auto* entity_health = em.get_component<components::health>(id);

	entity_health->current_health += amount;

	if (entity_health->current_health > entity_health->max_health) {
		entity_health->current_health = entity_health->max_health;
	}
}

void Health_System::activate_iframes(unsigned long long id, int i_frames) {
	if (!em.entities[id].mask.test(components::get_id<components::invincibility>())) {
		em.assign_component<components::invincibility>(id);
	}

	auto* invincibility_component = em.get_component<components::invincibility>(id);
	invincibility_component->max_duration = static_cast<double>(i_frames) / 60.0;
	invincibility_component->remaining_time = invincibility_component->max_duration;
}