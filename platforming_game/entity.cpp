#include "entity.h"

namespace components {
    int component_counter = 0; // Definition

    int health::id = get_id<health>(); // Definition
    int position::id = get_id<position>(); // Definition
    int movement::id = get_id<movement>(); // Definition
    int render::id = get_id<render>(); // Definition
    int gravity::id = get_id < gravity>();
    int input::id = get_id<input>();
    int damage::id = get_id<damage>();
    int regeneration::id = get_id<regeneration>();
    int thorns::id = get_id<thorns>();
    int pending_damage::id = get_id<pending_damage>();
    int invincibility::id = get_id<invincibility>();
}