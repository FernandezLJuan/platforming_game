#include "entity.h"

namespace components {
    int component_counter = 0; // Definition

    int health::id = get_id<health>(); // Definition
    int position::id = get_id<position>(); // Definition
    int movement::id = get_id<movement>(); // Definition
    int render::id = get_id<render>(); // Definition
    int gravity::id = get_id < gravity>();
    int input::id = get_id<input>();
}