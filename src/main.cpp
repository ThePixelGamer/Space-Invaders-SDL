#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"

#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstdint>

#include "si8080.h"

using namespace chrono; 
using frame = duration<int32_t, ratio<1, 60>>; 
using ms = duration<float, milli>; 

#define SCREEN_HEIGHT 256
#define SCREEN_WIDTH 224

#define CLOCK 2000000

SDL_Window*		window;
const Uint8*	state;
si8080* core = new si8080();
bool run = true;
bool vInterrupt = 1;

void keyboard(bool);
int main(int argc, char* args[]) {
	time_point<steady_clock> fpsTimer(steady_clock::now());
    if(argc > 1) {
		core->cmp = true;
		core->load(args[1]);
	}
	else {
		core->load("invaders.COM");
	}

	SDL_Renderer*				renderer;
	SDL_Texture*				texture;
	SDL_Event					event;
	
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);

	window = SDL_CreateWindow("Space Invaders", (DM.w/2)-(SCREEN_WIDTH/2), (DM.h/2)-(SCREEN_HEIGHT/2), SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    frame FPS{};

	//game in and out
	uint8_t x, y;
	while(run) {
		//Handle Updates
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			  	case SDL_KEYDOWN:
					state = SDL_GetKeyboardState(NULL);
					keyboard(true);
					break;

			  	case SDL_KEYUP:
					state = SDL_GetKeyboardState(NULL);
					keyboard(false);
					break;
			}
		}
		
		FPS = duration_cast<frame>(steady_clock::now() - fpsTimer);
		if(FPS.count() >= 1) {
			fpsTimer = steady_clock::now();
			SDL_SetWindowTitle(window, ("fps:" + to_string(60/FPS.count()) + " dt:" + to_string(duration_cast<ms>(FPS).count()) + " Invaders").c_str());

			uint32_t cycCount = 0;
			while(cycCount <= CLOCK / 60) {
				core->cycBefore = core->cycles;
				
				if(!core->hlt)
					core->emulateCycle();
				
				cycCount += core->cycles - core->cycBefore;
				
				if(core->cycles >= (CLOCK / 120)) {
					if(core->interrupt) {
						core->opcode = (vInterrupt) ? 0xcf : 0xd7;
						core->emulateCycle();
						core->cycles -= 11;
					}
					core->cycles -= (CLOCK / 120);
					vInterrupt = !vInterrupt;
				}

				if(core->opcode == 0xd3) { //write
					switch(core->loc) {
						case 2:
							core->port[2] = (x << (core->port[4] & 0x7)) + y; //prob something wrong
							break;
						case 4:
							y = x;
							x = core->port[5];
							break;
						default:
							cout << +core->loc << endl;
							break;
					}
				}
			}
		
			SDL_UpdateTexture(texture, NULL, core->pixels, SCREEN_WIDTH * sizeof(uint8_t) * 3);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
	}

	fclose(core->log); 

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

		if(state[SDL_SCANCODE_1]) core->port[1] = (core->port[1] ^ 0x4) + 0x4; //Player1 Start
		if(state[SDL_SCANCODE_2]) core->port[1] = (core->port[1] ^ 0x2) + 0x2; //Player2 Start
	}
	else {
		if(!state[SDL_SCANCODE_1]) core->port[1] ^= 0x4; //clear bit2 in port[1]
		if(!state[SDL_SCANCODE_2]) core->port[1] ^= 0x2; //clear bit1 in port[1]
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