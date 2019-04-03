#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"

#include <unistd.h>
#include <algorithm>
#include <iostream>

#include "si8080.h"

#define SCREEN_HEIGHT 256
#define SCREEN_WIDTH 224

#define CYCLES_EVERY_FRAME 2000000 / 60

SDL_Window*		window;
const Uint8*	state;
si8080* core = new si8080();
bool run = true;
bool vInterrupt = 1;

void keyboard(bool);
int main(int argc, char* args[]) {
	if(argc > 1) {
		core->cmp = true;
		cout << core->load(args[1]);
	}
	else {
		cout << core->load("invaders.COM");
	}

	SDL_Renderer*				renderer;
	SDL_Texture*				texture;
	SDL_Event					event;
	
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);

	window = SDL_CreateWindow("Space Invaders", (DM.w/2)-(SCREEN_WIDTH/2), (DM.h/2)-(SCREEN_HEIGHT/2), SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

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
			
				case SDL_WINDOWEVENT:
					core->drawFlag = true;
					break;

			  	default:
					break;
			}
		}
		
		core->cycCount = 0;
		
		while(core->cycCount <= CYCLES_EVERY_FRAME) {
			if(!core->hlt)
				core->emulateCycle(false);
			
			core->cycCount += core->cycles - core->cycBefore;
			
			if(core->cycles >= (CYCLES_EVERY_FRAME / 2)) {
				if(core->interrupt) {
					core->opcode = (vInterrupt) ? 0xd7 : 0xcf;
					vInterrupt = !vInterrupt;
					core->emulateCycle(true);		
					core->cycles += 4;
				}
				core->cycles -= (CYCLES_EVERY_FRAME / 2);
			}
		}
		
		if(core->drawFlag) {
			SDL_UpdateTexture(texture, NULL, core->pixels, SCREEN_WIDTH * sizeof(uint32_t));
			
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
			core->drawFlag = false;
		}
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

		if(state[SDL_SCANCODE_R])
			SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);

		// if(state[SDL_SCANCODE_1]) core->port[0] = (core->port[0] ^ 0x2) + 0x2; //READ1 bit1
		// if(state[SDL_SCANCODE_2]) core->port[0] = (core->port[0] ^ 0x4) + 0x4; //READ1 bit2
		// if(state[SDL_SCANCODE_SPACE]) core->port[0] = (core->port[0] ^ 0x10) + 0x10; //READ1 bit4
		// if(state[SDL_SCANCODE_LEFT]) core->port[0] = (core->port[0] ^ 0x20) + 0x20; //READ1 bit5
		// if(state[SDL_SCANCODE_RIGHT]) core->port[0] = (core->port[0] ^ 0x40) + 0x40; //READ1 bit6

		// if(state[SDL_SCANCODE_LSHIFT]) core->port[1] = (core->port[1] ^ 0x4) + 0x4; //READ2 bit2
		// if(state[SDL_SCANCODE_LALT]) core->port[1] = (core->port[1] ^ 0x10) + 0x10; //READ2 bit4
		// if(state[SDL_SCANCODE_COMMA]) core->port[1] = (core->port[1] ^ 0x20) + 0x20; //READ2 bit5
		// if(state[SDL_SCANCODE_PERIOD]) core->port[1] = (core->port[1] ^ 0x40) + 0x40; //READ2 bit6
	}
	else {
		// if(state[SDL_SCANCODE_1]) core->port[0] ^= 0x2; //READ1 bit1
		// if(state[SDL_SCANCODE_2]) core->port[0] ^= 0x4; //READ1 bit2
		// if(state[SDL_SCANCODE_SPACE]) core->port[0] ^= 0x10; //READ1 bit4
		// if(state[SDL_SCANCODE_LEFT]) core->port[0] ^= 0x20; //READ1 bit5
		// if(state[SDL_SCANCODE_RIGHT]) core->port[0] ^= 0x40; //READ1 bit6

		// if(state[SDL_SCANCODE_LSHIFT]) core->port[1] ^= 0x4; //READ2 bit2
		// if(state[SDL_SCANCODE_LALT]) core->port[1] ^= 0x10; //READ2 bit4
		// if(state[SDL_SCANCODE_COMMA]) core->port[1] ^= 0x20; //READ2 bit5
		// if(state[SDL_SCANCODE_PERIOD]) core->port[1] ^= 0x40; //READ2 bit6
	}
}

/*
		0x1  = 0000 0001
		0x2  = 0000 0010
		0x4  = 0000 0100
		0x8  = 0000 1000
		0x10 = 0001 0000
		0x20 = 0010 0000
		0x40 = 0100 0000
		0x80 = 1000 0000

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