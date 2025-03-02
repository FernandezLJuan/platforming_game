#pragma once
#include "entity.h"

class Health_System {
public:
	Health_System(entity_manager&);

	//updates health components of each entity with one
	void update(double);

private:
	void activate_iframes(unsigned long long, int);
	void damage_entity(unsigned long long, int); //reduce health of entity by an amount
	void heal_entity(unsigned long long, int); //heal entity by an amount

	entity_manager& em;
};