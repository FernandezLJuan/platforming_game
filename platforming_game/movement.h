#pragma once
#include <SDL3/SDL.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "input.h"
#include "entity.h"

class Collision_System;

class Collision_System {
public:
    enum collision_direction { NO_COLLISION, TOP_COLLISION, BOTTOM_COLLISION, LEFT_COLLISION, RIGHT_COLLISION };

	Collision_System(entity_manager&);
	collision_direction detect_collision(entity&, entity&);
	void resolve_collision(entity&, entity&, collision_direction);
	void resolve_rigid_collision(entity&, entity&, collision_direction);
	void resolve_health_damage(entity&, entity&);

private:
    entity_manager& em;
};

class Movement_System {
public:
	Movement_System(entity_manager& em, Input_Handler& input, Collision_System& collision_sys);

	void update(double);

private:
	entity_manager& em;
	Input_Handler& input;
};
