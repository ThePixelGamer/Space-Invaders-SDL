#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"

#include <unistd.h>
#include <algorithm>
#include <iostream>

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 224

SDL_Window*		window;
const Uint8*	state;

void keyboard(bool);
int main(int argc, char* args[]) {
	for (int i = 0; i < argc; ++i){
		if(strcmp(args[i], "-s") == 0){
			std::cout << "Set to server!" << std::endl;
		}
		if(strcmp(args[i], "-c") == 0){
			if(argc > i+1)
				std::cout << "Connecting to IP: " << args[i+1] << std::endl;
				//IP = args[i+1];
			else
				std::cout << "Connecting to default IP!" << std::endl;
		}
		if(strcmp(args[i], "-p") == 0){
			std::cout << "Setting Port To: " << args[i+1] << std::endl;
		}
	}

	SDL_Renderer*				renderer;
	SDL_Texture*				texture;
	SDL_Event					event;
	
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Space Invaders", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP); 
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	//myChip8 = new chip8();
	//myChip8->pixels = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT]; 
	//fill_n(myChip8->pixels, (SCREEN_WIDTH * SCREEN_HEIGHT), 0);
	
	//Load ROM
	
	while(true) {
		//Handle Updates
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					exit(0);
					break;
			  case SDL_KEYDOWN:
					state = SDL_GetKeyboardState(NULL);
					keyboard(true);
					break;
			  case SDL_KEYUP:
					state = SDL_GetKeyboardState(NULL);
					keyboard(false);
					break;
			  default:
					break;
			}
		}
		
		//Tick Engine
		
		//Push Render
		//SDL_UpdateTexture(texture, NULL, myChip8->pixels, SCREEN_WIDTH * sizeof(Uint32));
		
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Delay(2); //"vsync"
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window); 
	SDL_Quit(); 
	return 0;   
}

void keyboard(bool press){	
	if(press) {
		if(state[SDL_SCANCODE_ESCAPE])
			exit(0);

		if(state[SDL_SCANCODE_1])		myChip8->key[0x1] = 1;
		if(state[SDL_SCANCODE_2])		myChip8->key[0x2] = 1;
		if(state[SDL_SCANCODE_3])		myChip8->key[0x3] = 1;
		if(state[SDL_SCANCODE_4])		myChip8->key[0xC] = 1;

		if(state[SDL_SCANCODE_Q])		myChip8->key[0x4] = 1;
		if(state[SDL_SCANCODE_W])		myChip8->key[0x5] = 1;
		if(state[SDL_SCANCODE_E])		myChip8->key[0x6] = 1;
		if(state[SDL_SCANCODE_R])		myChip8->key[0xD] = 1;

		if(state[SDL_SCANCODE_A])		myChip8->key[0x7] = 1;
		if(state[SDL_SCANCODE_S])		myChip8->key[0x8] = 1;
		if(state[SDL_SCANCODE_D])		myChip8->key[0x9] = 1;
		if(state[SDL_SCANCODE_F])		myChip8->key[0xE] = 1;

		if(state[SDL_SCANCODE_Z])		myChip8->key[0xA] = 1;
		if(state[SDL_SCANCODE_X])		myChip8->key[0x0] = 1;
		if(state[SDL_SCANCODE_C])		myChip8->key[0xB] = 1;
		if(state[SDL_SCANCODE_V])		myChip8->key[0xF] = 1;
	}
	else {
		if(!(state[SDL_SCANCODE_1]))	myChip8->key[0x1] = 0;
		if(!(state[SDL_SCANCODE_2]))	myChip8->key[0x2] = 0;
		if(!(state[SDL_SCANCODE_3]))	myChip8->key[0x3] = 0;
		if(!(state[SDL_SCANCODE_4]))	myChip8->key[0xC] = 0;

		if(!(state[SDL_SCANCODE_Q]))	myChip8->key[0x4] = 0;
		if(!(state[SDL_SCANCODE_W]))	myChip8->key[0x5] = 0;
		if(!(state[SDL_SCANCODE_E]))	myChip8->key[0x6] = 0;
		if(!(state[SDL_SCANCODE_R]))	myChip8->key[0xD] = 0;

		if(!(state[SDL_SCANCODE_A]))	myChip8->key[0x7] = 0;
		if(!(state[SDL_SCANCODE_S]))	myChip8->key[0x8] = 0;
		if(!(state[SDL_SCANCODE_D]))	myChip8->key[0x9] = 0;
		if(!(state[SDL_SCANCODE_F]))	myChip8->key[0xE] = 0;

		if(!(state[SDL_SCANCODE_Z]))	myChip8->key[0xA] = 0;
		if(!(state[SDL_SCANCODE_X]))	myChip8->key[0x0] = 0;
		if(!(state[SDL_SCANCODE_C]))	myChip8->key[0xB] = 0;
		if(!(state[SDL_SCANCODE_V]))	myChip8->key[0xF] = 0;
	}
}