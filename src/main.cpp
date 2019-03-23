#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"

#include <unistd.h>
#include <algorithm>
#include <iostream>

#include "si8080.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 224

SDL_Window*		window;
const Uint8*	state;
si8080* core = new si8080();
bool run = true;
bool isGoingOnline = false;

void keyboard(bool);
int main(int argc, char* args[]) {
	for (int i = 0; i < argc; ++i){
		if(strcmp(args[i], "-s") == 0){
			std::cout << "Set to server!" << std::endl;
			isGoingOnline = true;
		}
		if(strcmp(args[i], "-c") == 0){
			isGoingOnline = true;
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

	window = SDL_CreateWindow("Space Invaders", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	core->pixels = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];
	std::fill_n(core->pixels, (SCREEN_WIDTH * SCREEN_HEIGHT), 0);

	//Load ROM
	
	while(run) {
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

		if(isGoingOnline){
			//server
				//Grab Client Inputs
				
				//Handle Client Inputs

				//Tick
				std::string out = core->emulateCycle();

				//Send opcode to client

			//client
		} else {
			core->emulateCycle();
		}
		
		//Push Render
		if(core->drawFlag)
			SDL_UpdateTexture(texture, NULL, core->pixels, SCREEN_WIDTH * sizeof(Uint32));
		
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
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
			run = false; //Much Safer Exit :)

		/*
		0	coin (0 when active)
		1	P2 start button
		2	P1 start button
		3	?
		4	P1 shoot button
		5	P1 joystick left
		6	P1 joystick right
		7	?
	
		0,1	dipswitch number of lives (0:3,1:4,2:5,3:6)
		2	tilt 'button'
		3	dipswitch bonus life at 1:1000,0:1500
		4	P2 shoot button
		5	P2 joystick left
		6	P2 joystick right
		7	dipswitch coin info 1:off,0:on
		*/
		if(state[SDL_SCANCODE_1]); //READ1 bit1
		if(state[SDL_SCANCODE_2]); //READ1 bit2
		if(state[SDL_SCANCODE_SPACE]); //READ1 bit4
		if(state[SDL_SCANCODE_LEFT]); //READ1 bit5
		if(state[SDL_SCANCODE_RIGHT]); //READ1 bit6

		if(state[SDL_SCANCODE_LSHIFT]); //READ2 bit2
		if(state[SDL_SCANCODE_LALT]); //READ2 bit4
		if(state[SDL_SCANCODE_COMMA]); //READ2 bit5
		if(state[SDL_SCANCODE_PERIOD]); //READ2 bit6
	}
	else {
		if(state[SDL_SCANCODE_1]); //READ1 bit1
		if(state[SDL_SCANCODE_2]); //READ1 bit2
		if(state[SDL_SCANCODE_SPACE]); //READ1 bit4
		if(state[SDL_SCANCODE_LEFT]); //READ1 bit5
		if(state[SDL_SCANCODE_RIGHT]); //READ1 bit6

		if(state[SDL_SCANCODE_LSHIFT]); //READ2 bit2
		if(state[SDL_SCANCODE_LALT]); //READ2 bit4
		if(state[SDL_SCANCODE_COMMA]); //READ2 bit5
		if(state[SDL_SCANCODE_PERIOD]); //READ2 bit6
	}
}