#pragma once
#include <SDL3/SDL.h>
#include <functional>
#include <array>

struct key {
	bool pressed{ false };
	bool released{ false };
	double pressed_time{ 0.0 };
	double last_duration{ 0.0 };

	std::function<void()> action; //action to execute when the key is pressed
};

class Input_Handler {
public:
	Input_Handler() : quit(false){
		assign_action(SDL_SCANCODE_ESCAPE, [this]() {quit = true; });
	}

	bool should_quit() {
		return quit;
	}

	bool should_pause() {
		return is_key_pressed(SDL_SCANCODE_P);
	}

	void check_input() {
		for (auto& k : keys) {
			k.released = false;
		}

		while (SDL_PollEvent(&keyboard_event)) {

			if (keyboard_event.type == SDL_EVENT_QUIT) {
				quit = true;
			}

			if (keyboard_event.type == SDL_EVENT_KEY_DOWN) {
				unsigned int key_code = keyboard_event.key.scancode;

				if (!keys[key_code].pressed) {
					//if the key was not pressed, set it's state as pressed
					keys[key_code].pressed = true;
					keys[key_code].pressed_time = static_cast<double>(SDL_GetTicks());
				}

				//if the key as an assigned action, execute it when the key is pressed
				if (keys[key_code].action) {
					keys[key_code].action();
				}
			}
			
			if (keyboard_event.type == SDL_EVENT_KEY_UP) {
				//set key state as not pressed and compute total pressed time
				unsigned int key_code = keyboard_event.key.scancode;

				keys[key_code].pressed = false;
				keys[key_code].released = true;
				keys[key_code].last_duration = SDL_GetTicks() - keys[key_code].pressed_time;

			}
		}
	}

	double key_time(unsigned int scancode) {
		if (is_key_pressed(scancode)) {
			return SDL_GetTicks() - keys[scancode].pressed_time ;
		}

		return 0.0;
	}

	double last_pressed_time(unsigned int scancode) {
		return keys[scancode].last_duration;
	}

	void assign_action(unsigned int scancode, std::function<void()> action) {
		if (!keys[scancode].action) {
			keys[scancode].action = action;
		}
	}

	bool is_key_pressed(unsigned int scancode) {
		return keys[scancode].pressed;
	}

	bool is_key_released(unsigned int scancode) {
		return keys[scancode].released;
	}

private:

	SDL_Event keyboard_event;
	std::array<key, 256> keys;
	bool quit;
};