#pragma once
#include <SDL3/SDL.h>
#include <iostream>
#include "movement.h"
#include "input.h"

class Game {
public:
	Game();
	~Game() {
		cleanup();
	}

	void run();

private:
	void init();
	void cleanup();
	void handle_input();
	void update(double);
	void render();

	bool is_running;
	bool paused;

	SDL_Window* window;
	SDL_Renderer* renderer;

	entity_manager em;
	Input_Handler input;

	Movement_System movement_system;
	Collision_System collision;
	//Render_System render_system;
};