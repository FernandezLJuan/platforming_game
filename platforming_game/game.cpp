#include "game.h"

Game::Game() : is_running(true), window(nullptr), renderer(nullptr), collision(em), movement_system(em, input, collision), health_system(em) {
	init();
}

void Game::init() {
	if (SDL_Init(SDL_INIT_VIDEO) == 0) {
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
		is_running = false;
		return;
	}

	window = SDL_CreateWindow("game", 900, 900, SDL_WINDOW_RESIZABLE);
	if (!window) {
		std::cerr << "Could not create SDL_Window: " << SDL_GetError() << "\n";
		is_running = false;
		return;
	}

	renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer) {
		std::cerr << "Could not create SDL_Renderer: " << SDL_GetError() << "\n";
		is_running = false;
		return;
	}

	create_player();
	create_enemy();

	//ground doesn't need movement or input components, only collisions and renders
	unsigned long long ground_id = em.new_entity();
	em.assign_component<components::position>(ground_id);
	em.assign_component<components::collision>(ground_id);
	em.assign_component<components::render>(ground_id);

	auto* ground_position = em.get_component<components::position>(ground_id);
	ground_position->pos.x = -500;
	ground_position->pos.y = 600;

	auto* ground_render = em.get_component<components::render>(ground_id);
	ground_render->sprite_rect = { static_cast<float>(ground_position->pos.x), static_cast<float>(ground_position->pos.y), 1900,200};
	ground_render->original_width = 1900;
	ground_render->render_color = {0x00,0x00,0xFF,0xFF};

	auto* ground_collision = em.get_component<components::collision>(ground_id);
	ground_collision->hitbox = ground_render->sprite_rect;
	ground_collision->is_rigid = true;

	unsigned long long wall_id = em.new_entity();
	em.assign_component<components::position>(wall_id);
	em.assign_component<components::collision>(wall_id);
	em.assign_component<components::render>(wall_id);

	auto* wall_position = em.get_component<components::position>(wall_id);
	wall_position->pos.x = 800;
	wall_position->pos.y = 0;

	auto* wall_render = em.get_component<components::render>(wall_id);
	wall_render->sprite_rect = { static_cast<float>(wall_position->pos.x), static_cast<float>(wall_position->pos.y), 200,555 };
	wall_render->original_width = 200;
	wall_render->render_color = { 0x00,0xFF,0xFF,0xFF };

	auto* wall_collision = em.get_component<components::collision>(wall_id);
	wall_collision->hitbox = wall_render->sprite_rect;
	wall_collision->is_rigid = true;

	unsigned long long death_ground = em.new_entity();
	em.assign_component<components::collision>(death_ground);
	em.assign_component<components::damage>(death_ground);
	auto* death_collision = em.get_component<components::collision>(death_ground);
	death_collision->hitbox = { -1000, 800, 5000, 200 };
	auto* death_damage = em.get_component<components::damage>(death_ground);
	death_damage->damage_amount = 20;

	death_collision->is_rigid = true;

	std::cout << "Death ID: " << death_ground << "\n";
}

void Game::create_player() {
	unsigned long long player_id = em.new_entity();
	em.assign_component<components::position>(player_id);
	em.assign_component<components::movement>(player_id);
	em.assign_component<components::render>(player_id);
	em.assign_component<components::gravity>(player_id);
	em.assign_component<components::input>(player_id);
	em.assign_component<components::collision>(player_id);
	em.assign_component<components::health>(player_id);
	em.assign_component<components::jump>(player_id);

	auto* player_position = em.get_component<components::position>(player_id);
	player_position->pos = { 10.0,10.0 };

	auto* player_movement = em.get_component<components::movement>(player_id);
	player_movement->speed = { 0.0,0.0 };
	player_movement->acceleration = { 2.0f,4.0f };

	player_movement->max_speed = { 15.0,50.0 };
	player_movement->max_acceleration = { 5.0,5.0 };

	auto* player_sprite = em.get_component<components::render>(player_id);
	player_sprite->sprite_rect = { static_cast<float>(player_position->pos.x), static_cast<float>(player_position->pos.y), 50,50 };
	player_sprite->original_width = 50;
	player_sprite->render_color = { 0x00,0xFF,0x00,0xFF };

	auto* player_collision = em.get_component<components::collision>(player_id);
	player_collision->hitbox.x = player_position->pos.x;
	player_collision->hitbox.y = player_position->pos.y;
	player_collision->hitbox.w = player_sprite->sprite_rect.w;
	player_collision->hitbox.h = player_sprite->sprite_rect.h;

	auto* player_health = em.get_component <components::health>(player_id);
	player_health->max_health = 100;
	player_health->current_health = player_health->max_health;
	player_health->i_frames = 5;
}

void Game::create_enemy() {
	//create enemy entity
	unsigned long long enemy_id = em.new_entity();
	em.assign_component<components::position>(enemy_id);
	em.assign_component<components::render>(enemy_id);
	em.assign_component<components::movement>(enemy_id);
	em.assign_component<components::gravity>(enemy_id);
	em.assign_component<components::collision>(enemy_id);
	em.assign_component<components::damage>(enemy_id);

	auto* enemy_position = em.get_component<components::position>(enemy_id);
	enemy_position->pos.x = 500;
	enemy_position->pos.y = 10;

	auto* enemy_movement = em.get_component<components::movement>(enemy_id);
	enemy_movement->speed = { 0.0,0.0 };
	enemy_movement->acceleration = { 5.0f,5.0f };

	enemy_movement->max_speed = { 100.0,100.0 };
	enemy_movement->max_acceleration = { 5.0,5.0 };

	auto* enemy_render = em.get_component<components::render>(enemy_id);
	enemy_render->sprite_rect = { static_cast<float>(enemy_position->pos.x), static_cast<float>(enemy_position->pos.y), 30, 30 };

	auto* enemy_collision = em.get_component<components::collision>(enemy_id);
	enemy_collision->hitbox.x = enemy_position->pos.x;
	enemy_collision->hitbox.y = enemy_position->pos.y;
	enemy_collision->hitbox.w = enemy_render->sprite_rect.w;
	enemy_collision->hitbox.h = enemy_render->sprite_rect.h;
	enemy_collision->is_rigid = false;

	auto* enemy_damage = em.get_component<components::damage>(enemy_id);
	enemy_damage->damage_amount = 5;
}

void Game::cleanup() {
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;

	SDL_DestroyWindow(window);
	window = nullptr;
	 
	SDL_Quit();
}

void Game::run() {
	double delta_time = 0.0;
	unsigned int last_time = 0;
	unsigned int current_time = SDL_GetPerformanceCounter();

	while (is_running) {
		last_time = current_time;
		current_time = SDL_GetPerformanceCounter();

		delta_time = (double)(current_time - last_time) / SDL_GetPerformanceFrequency();
		
		handle_input();

		if(!paused) 
			update(delta_time);

		render();

		SDL_Delay(16); //60 fps
	}
}

void Game::handle_input() {
	input.check_input();

	if (input.should_quit()) {
		is_running = false;
	}

}

void Game::update(double delta_time) {
	movement_system.update(delta_time);

	for (auto& e : em.entities) {
		for (auto& other_entity : em.entities) {
			if (e.id == other_entity.id) {
				continue;
			}
			if (other_entity.mask.test(components::get_id<components::collision>())) {	
				collision.detect_collision(e,other_entity);
			}
		}
	}

	health_system.update(delta_time);
}

void Game::render() {
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	for (auto& e : em.entities) {
		if (e.mask.test(components::get_id<components::render>())) {
			auto* entity_sprite = em.get_component<components::render>(e.id);
			auto* entity_position = em.get_component<components::position>(e.id);
			SDL_Color render_color = entity_sprite->render_color;

			entity_sprite->sprite_rect.x = entity_position->pos.x;
			entity_sprite->sprite_rect.y = entity_position->pos.y;

			SDL_SetRenderDrawColor(renderer, render_color.r, render_color.b, render_color.g, render_color.a);
			SDL_RenderFillRect(renderer, &entity_sprite->sprite_rect);

			if (e.mask.test(components::get_id < components::health>())) {
				auto* entity_health = em.get_component<components::health>(e.id);
				
				SDL_FRect health_bar{ static_cast<float>(entity_position->pos.x), static_cast<float>(entity_position->pos.y - 30), entity_health->current_health, 10 };

				//draw red health bar of entity on top of it
				SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
				SDL_RenderFillRect(renderer, &health_bar);
			}
		}
	}

	SDL_RenderPresent(renderer);
}