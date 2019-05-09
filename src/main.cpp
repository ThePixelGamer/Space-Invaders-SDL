#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_mixer.h"

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
#define FRAME 33323 //2000000 / 60 - 11 (for rst)
#define HALFFRAME 16656 //2000000 / 120 - 11 (for rst)

SDL_Window*		window;
SDL_Renderer*	renderer;
SDL_Texture*	texture;
SDL_Event		event;
const Uint8*	state;
SDL_DisplayMode dm;
Mix_Chunk		*wav1, *wav2, *wav3, *wav4, *wav5, *wav6, *wav7, *wav8, *wav9, *wav10;

si8080* core = new si8080();
bool run = true, vInterrupt = true;
uint8_t port1 = 0b1000, port2 = 0b1000, x, y, offset;

void keyboard(bool);
int main(int argc, char* args[]) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_GetCurrentDisplayMode(0, &dm);

	window = SDL_CreateWindow("Space Invaders", (dm.w/2)-(SCREEN_WIDTH/2), (dm.h/2)-(SCREEN_HEIGHT/2), SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
	Mix_Volume(-1, MIX_MAX_VOLUME/8);
	wav1 = Mix_LoadWAV("./sounds/spaceship_move.wav");	
	wav2 = Mix_LoadWAV("./sounds/player_shot.wav");
	wav3 = Mix_LoadWAV("./sounds/player_hit.wav");
	wav4 = Mix_LoadWAV("./sounds/invaders_hit.wav");
	wav5 = Mix_LoadWAV("./sounds/spaceship_hit.wav");
	wav6 = Mix_LoadWAV("./sounds/invaders_move1.wav");
	wav7 = Mix_LoadWAV("./sounds/invaders_move2.wav");
	wav8 = Mix_LoadWAV("./sounds/invaders_move3.wav");
	wav9 = Mix_LoadWAV("./sounds/invaders_move4.wav");
	wav10 = Mix_LoadWAV("./sounds/spaceship_hit.wav"); //extra life

	if(argc > 1) {
		core->load(args[1]);
	} else {
		core->load("invaders.com");
	}

	time_point<steady_clock> fpsTimer(steady_clock::now());
	frame FPS{};
	while(run) {
		FPS = duration_cast<frame>(steady_clock::now() - fpsTimer);
		if(FPS.count() >= 1) {
			fpsTimer = steady_clock::now();
			SDL_SetWindowTitle(window, ("fps:" + to_string(60/FPS.count()) + " Invaders").c_str());

			uint32_t cycCount = 0;
			while(cycCount <= FRAME) {
				while(SDL_PollEvent(&event)) { //user input
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

				if(core->opcode == 0xdb) { //game input
					switch(core->loc) {
						case 1:
							core->portIn[0] &= port1;
						break;
						case 2: 
							core->portIn[1] &= port2;
						break; 
						case 3:	
							core->portIn[2] = (x << offset) + (y >> (8 - offset));
						break;

						default: cout << +core->loc << endl; break;
					}
				}
				if(core->opcode == 0xd3) { //game output
					switch(core->loc) {
						case 2:
							offset = core->portOut[0];
						break;
						case 4:
							y = x;
							x = core->portOut[2];
						break;

						case 3: 
							if(core->soundB) {
								if((core->portOut[1] & 0x1) != 0) 
									Mix_PlayChannel(-1, wav1, 0);
								if((core->portOut[1] & 0x2) != 0)
									Mix_PlayChannel(-1, wav2, 0);
								if((core->portOut[1] & 0x4) != 0)
									Mix_PlayChannel(-1, wav3, 0);
								if((core->portOut[1] & 0x8) != 0)
									Mix_PlayChannel(-1, wav4, 0);
								if((core->portOut[1] & 0x10) != 0)
									Mix_PlayChannel(-1, wav5, 0);
							}
						break;
						case 5: 
							core->soundB = (core->portOut[3] >> 5) & 0x1;
							if(core->soundB) {
								if((core->portOut[3] & 0x1) != 0)
									Mix_PlayChannel(-1, wav6, 0);
								if((core->portOut[3] & 0x2) != 0)
									Mix_PlayChannel(-1, wav7, 0);
								if((core->portOut[3] & 0x4) != 0)
									Mix_PlayChannel(-1, wav8, 0);
								if((core->portOut[3] & 0x8) != 0)
									Mix_PlayChannel(-1, wav9, 0);
								if((core->portOut[3] & 0x10) != 0)
									Mix_PlayChannel(-1, wav10, 0);
							}
						break;

						case 6: break; //writes A to this port, debug thing?
						default: cout << +core->loc << endl; break;
					}
				}

				if(core->hltB)
					core->cycles += 4;
				else
					core->emulateCycle();

				cycCount += core->cycles - core->cycBefore;
				if(core->cycles >= HALFFRAME) {
					if(core->interruptB) {
						core->pc -= 3;
						uint16_t pctmp = core->pc;
						uint8_t optmp = core->memory[pctmp];
						core->memory[pctmp] = ((vInterrupt) ? 0xcf : 0xd7);
						core->emulateCycle();
						core->memory[pctmp] = optmp;
					}
					
					core->cycles -= HALFFRAME;
					vInterrupt = !vInterrupt;
				}
			}

			SDL_UpdateTexture(texture, NULL, core->pixels, SCREEN_WIDTH * sizeof(uint8_t) * 3);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
	}

	if(core->debugB)
		fclose(core->log); 

	Mix_FreeChunk(wav1);
	Mix_FreeChunk(wav2);
	Mix_FreeChunk(wav3);
	Mix_FreeChunk(wav4);
	Mix_FreeChunk(wav5);
	Mix_FreeChunk(wav6);
	Mix_FreeChunk(wav7);
	Mix_FreeChunk(wav8);
	Mix_FreeChunk(wav9);
	Mix_FreeChunk(wav10);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window); 
	SDL_Quit(); 
	return 0;   
}

void keyboard(bool press) {	
	if(press) {
		if(state[SDL_SCANCODE_ESCAPE]) 	run = false;
		if(state[SDL_SCANCODE_R]) 		SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
		if(state[SDL_SCANCODE_C]) 		port1 = core->portIn[0] = (port1 & 0b11111110) + 0b0001; //Coin Deposit :D
		if(state[SDL_SCANCODE_1]) 		port1 = core->portIn[0] = (port1 & 0b11111011) + 0b0100; //Player1 Start
		if(state[SDL_SCANCODE_2]) 		port1 = core->portIn[0] = (port1 & 0b11111101) + 0b0010; //Player2 Start
		if(state[SDL_SCANCODE_SPACE]) 	port1 = core->portIn[0] = (port1 & 0b11101111) + 0b00010000; //Player1 Shot
		if(state[SDL_SCANCODE_A]) 		port1 = core->portIn[0] = (port1 & 0b11011111) + 0b00100000; //Player1 Left
		if(state[SDL_SCANCODE_D]) 		port1 = core->portIn[0] = (port1 & 0b10111111) + 0b01000000; //Player1 Right
		if(state[SDL_SCANCODE_KP_0]) 	port2 = core->portIn[1] = (port2 & 0b11101111) + 0b00010000; //Player2 Shot
		if(state[SDL_SCANCODE_KP_4]) 	port2 = core->portIn[1] = (port2 & 0b11011111) + 0b00100000; //Player2 Left
		if(state[SDL_SCANCODE_KP_6]) 	port2 = core->portIn[1] = (port2 & 0b10111111) + 0b01000000; //Player2 Right
	} else {
		if(!state[SDL_SCANCODE_C]) 		port1 = port1 & 0b11111110; //clear bit0 in portIn[0]
		if(!state[SDL_SCANCODE_1]) 		port1 = port1 & 0b11111011; //clear bit2 in portIn[0]
		if(!state[SDL_SCANCODE_2]) 		port1 = port1 & 0b11111101; //clear bit1 in portIn[0]
		if(!state[SDL_SCANCODE_SPACE]) 	port1 = port1 & 0b11101111; //clear bit4 in portIn[0]
		if(!state[SDL_SCANCODE_A]) 		port1 = port1 & 0b11011111; //clear bit5 in portIn[0]
		if(!state[SDL_SCANCODE_D]) 		port1 = port1 & 0b10111111; //clear bit6 in portIn[0]
		if(!state[SDL_SCANCODE_KP_0]) 	port2 = port2 & 0b11101111; //clear bit4 in portIn[1]
		if(!state[SDL_SCANCODE_KP_4]) 	port2 = port2 & 0b11011111; //clear bit5 in portIn[1]
		if(!state[SDL_SCANCODE_KP_6])	port2 = port2 & 0b10111111; //clear bit6 in portIn[1]
	}
}