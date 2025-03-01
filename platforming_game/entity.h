#pragma once
#include <SDL3/SDL.h>
#include <bitset>
#include <vector>

#define MAX_COMPONENTS 32

namespace types {
    template<typename T>
    struct Vec2 {
        T x;
        T y;
    };
}

enum class collision_types {
    RIGID_BODY,
    DAMAGE
};

namespace components {
    extern int component_counter;

    template<class T>
    int get_id() {
        static int component_id = component_counter++;
        return component_id;
    }

    struct health {
        static int id;
        const int max_health;
        const int current_health;

        health(int max, int current) : max_health(max), current_health(current) {}
    };

    struct position {
        static int id; // Declaration
        types::Vec2<double> pos{ 0,0 };
    };

    struct movement {
        static int id; // Declaration
        types::Vec2<double> speed{ 0,0 };
        types::Vec2<double> acceleration{ 0,0 };
        types::Vec2<double> max_speed{ 0,0 };
        types::Vec2<double> max_acceleration{ 0,0 };

        types::Vec2<double> deceleration{ 70.0,70.0 };
    };

    struct render {
        static int id; // Declaration
        SDL_FRect sprite_rect{ 0,0,10,10 };
        int original_width = 10;
        SDL_Color render_color = { 0xFF,0x00,0x00,0xFF };
    };

    struct physics {
        static int id;
        double x_forces = 0.0; //sum of all forces acting in the x axis 
        double y_forces = 0.0; //sum of all forces acting in the y axis (normal force, gravity, etc.)
        double mass; //mass of an entity, entities with 0 mass will not be affected by gravity (obviously)
        double friction = 0.3; //friction coefficient, determines how much speed is reduced by friction with a surface
    };

    struct gravity {
        static int id;
        double gravity_constant = 9.8;
    };
    
    struct jump {
        static int id;
        bool is_grounded = false;
        double jump_force = -200.0;
    };

    struct input {
        static int id;
        unsigned int move_left = SDL_SCANCODE_A;
        unsigned int move_right = SDL_SCANCODE_D;
        unsigned int jump = SDL_SCANCODE_SPACE;
        unsigned int crouch = SDL_SCANCODE_S;
    };

    struct collision {
        static int id;
        SDL_FRect hitbox;
        collision_types collision_nature = collision_types::RIGID_BODY;
    };
}

struct entity {
    unsigned long long id;
    std::bitset<MAX_COMPONENTS> mask; //bitmask to identify components
};

struct component_pool {
   component_pool(size_t e_size) : element_size(e_size) {
        component_array = new char[element_size * MAX_COMPONENTS];
   }

   ~component_pool() {
       delete[] component_array;
   }

   inline void* get(int id) {
       return component_array + id * element_size;
   }

    size_t element_size;
    char* component_array;
};

struct entity_manager {
    entity_manager() {}

    unsigned long long new_entity() {
        entities.push_back(entity{ entities.size(), std::bitset<MAX_COMPONENTS>() });
        return entities.back().id;
    }

    template<class T>
    T* assign_component(unsigned long long id) {
        int component_id = components::get_id<T>();

        if (components_pool.size() <= component_id) {
            components_pool.resize(component_id + 1, nullptr);
        }

        if (components_pool[component_id] == nullptr) {
            components_pool[component_id] = new component_pool(sizeof(T));
        }

        T* component = new (components_pool[component_id]->get(id)) T();

        entities[id].mask.set(component_id);
        return component;
    }

    template<class T>
    void remove_component(unsigned long long id) {
        int component_id = components::get_id<T>();
        entities[id].mask.reset(component_id);
    }

    template<class T>
    T* get_component(unsigned long long id) {
        int component_id = components::get_id<T>();
        if (!entities[id].mask.test(component_id)) return nullptr;
        return static_cast<T*>(components_pool[component_id]->get(id));
    }

    std::vector<entity> entities;
    std::vector<component_pool*> components_pool;
};