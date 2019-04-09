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
		core->cpmB = true;
		core->load(args[1]);
	}
	else {
		core->load("invaders.com");
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

		if(!core->cpmB) {

			FPS = duration_cast<frame>(steady_clock::now() - fpsTimer);
			if(FPS.count() >= 1) {
				fpsTimer = steady_clock::now();
				SDL_SetWindowTitle(window, ("fps:" + to_string(60/FPS.count()) + " dt:" + to_string(duration_cast<ms>(FPS).count()) + " Invaders").c_str());

				uint32_t cycCount = 0;
				while(cycCount <= CLOCK / 60) {
					core->cycBefore = core->cycles;
					
					if(!core->hlt)
						core->emulateCycle();
					
					if(core->opcode == 0xd3) { //handle output
						switch(core->loc) {
							case 2:
								core->portIn[2] = (x << (core->portOut[0] & 0x7)) + (y >> (8 - (core->portOut[0] & 0x7))); //prob something wrong
								break;
							case 3: break; //samples + extended play
							case 4:
								y = x;
								x = core->portOut[2];
								break;
							case 5: break; //samples + amp?
							case 6: break; //writes A to this port, debug thing?
							default:
								cout << +core->loc << endl;
								break;
						}
					}
					if((core->opcode == 0xdb) && (keyupB == true)) {
						//copy keyups to input ports
					}

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

					while(SDL_PollEvent(&event)) { //handle input
						switch(event.type) {
							case SDL_KEYDOWN:
								state = SDL_GetKeyboardState(NULL);
								keyboard(true);
								break;

							case SDL_KEYUP:
								state = SDL_GetKeyboardState(NULL);
								//make a copy of the keyups (could use the existing keyboard function)
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
		else {
			if(!core->hlt)
				core->emulateCycle();

			core->cycles = 0;
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

		if(state[SDL_SCANCODE_C]) 			core->portIn[0] = (core->portIn[0] & 0b11111110) + 0b0001; //Coin Deposit :D
		if(state[SDL_SCANCODE_1]) 			core->portIn[0] = (core->portIn[0] & 0b11111011) + 0b0100; //Player1 Start
		if(state[SDL_SCANCODE_2]) 			core->portIn[0] = (core->portIn[0] & 0b11111101) + 0b0010; //Player2 Start
		if(state[SDL_SCANCODE_SPACE]) 		core->portIn[0] = (core->portIn[0] & 0b11101111) + 0b00010000; //Player1 Shot
		if(state[SDL_SCANCODE_A]) 			core->portIn[0] = (core->portIn[0] & 0b11011111) + 0b00100000; //Player1 Left
		if(state[SDL_SCANCODE_D]) 			core->portIn[0] = (core->portIn[0] & 0b10111111) + 0b01000000; //Player1 Right
		if(state[SDL_SCANCODE_KP_0]) 		core->portIn[1] = (core->portIn[1] & 0b11101111) + 0b00010000; //Player2 Shot
		if(state[SDL_SCANCODE_KP_4]) 		core->portIn[1] = (core->portIn[1] & 0b11011111) + 0b00100000; //Player2 Left
		if(state[SDL_SCANCODE_KP_6]) 		core->portIn[1] = (core->portIn[1] & 0b10111111) + 0b01000000; //Player2 Right
	}
	else {
		if(!state[SDL_SCANCODE_C]) 		core->portIn[0] &= 0b11111110; //clear bit0 in portIn[0]
		if(!state[SDL_SCANCODE_1]) 		core->portIn[0] &= 0b11111011; //clear bit2 in portIn[0]
		if(!state[SDL_SCANCODE_2]) 		core->portIn[0] &= 0b11111101; //clear bit1 in portIn[0]
		if(!state[SDL_SCANCODE_SPACE]) 	core->portIn[0] &= 0b11101111; //clear bit4 in portIn[0]
		if(!state[SDL_SCANCODE_A]) 		core->portIn[0] &= 0b11011111; //clear bit5 in portIn[0]
		if(!state[SDL_SCANCODE_D]) 		core->portIn[0] &= 0b10111111; //clear bit6 in portIn[0]
		if(!state[SDL_SCANCODE_KP_0]) 	core->portIn[1] &= 0b11101111; //clear bit4 in portIn[1]
		if(!state[SDL_SCANCODE_KP_4]) 	core->portIn[1] &= 0b11011111; //clear bit5 in portIn[1]
		if(!state[SDL_SCANCODE_KP_6]) 	core->portIn[1] &= 0b10111111; //clear bit6 in portIn[1]
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