#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_mixer.h"

#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <cmath>

#include "si8080.h"

using namespace chrono; 
using frame = duration<int32_t, ratio<1, 60>>; 
using ms = duration<float, milli>; 

#define SCREEN_HEIGHT 256
#define SCREEN_WIDTH 224
#define CLOCK 2000000

SDL_Window*		window;
SDL_Renderer*	renderer;
SDL_Texture*	texture;
SDL_Event		event;
SDL_DisplayMode dm;
// Mix_Chunk		*wav1, *wav2, *wav3, *wav4, *wav5, *wav6, *wav7, *wav8, *wav9, *wav10;

si8080* core = new si8080();
bool vInterrupt = true;
uint8_t port1 = 0b1000, port2 = 0b1000, x, y, offset;
uint32_t cycCount = 0, fpsI = 60;

void keyboard(bool);
int main(int argc, char* args[]) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_GetCurrentDisplayMode(0, &dm);

	window = SDL_CreateWindow("Space Invaders", (dm.w/2)-(SCREEN_WIDTH/2), (dm.h/2)-(SCREEN_HEIGHT/2), SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, 0);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
	// Mix_Volume(-1, MIX_MAX_VOLUME/8);
	// wav1 = Mix_LoadWAV("./sounds/spaceship_move.wav");	
	// wav2 = Mix_LoadWAV("./sounds/player_shot.wav");
	// wav3 = Mix_LoadWAV("./sounds/player_hit.wav");
	// wav4 = Mix_LoadWAV("./sounds/invaders_hit.wav");
	// wav5 = Mix_LoadWAV("./sounds/spaceship_hit.wav");
	// wav6 = Mix_LoadWAV("./sounds/invaders_move1.wav");
	// wav7 = Mix_LoadWAV("./sounds/invaders_move2.wav");
	// wav8 = Mix_LoadWAV("./sounds/invaders_move3.wav");
	// wav9 = Mix_LoadWAV("./sounds/invaders_move4.wav");
	// wav10 = Mix_LoadWAV("./sounds/spaceship_hit.wav"); //extra life

	if(argc > 1) {
		core->cpmB = true;
		core->runB = true;
		core->load(args[1]);
	} else {
		core->cpmB = false;
		core->load("invaders.com");
	}

	SDL_UpdateTexture(texture, NULL, core->pixels, SCREEN_WIDTH * sizeof(uint8_t) * 3);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	double cycPerFrame = round((double)CLOCK / fpsI);
	double everyHalfCPF = round((double)CLOCK / (fpsI * 2));

	time_point<steady_clock> fpsTimer(steady_clock::now());
	frame fps{};
	while(core->runB) {
		fps = duration_cast<frame>(steady_clock::now() - fpsTimer);
		if(fps.count() >= 1) {
			fpsTimer = steady_clock::now();
			cycCount = 0;

			while(SDL_PollEvent(&event) != 0) { //user input
				// switch(event.type) {
				// 	case SDL_QUIT: core->runB = false; break;
				// 	case SDL_KEYDOWN: keyboard(true); break;
				// 	case SDL_KEYUP: keyboard(false); break;
				// }
			}

			if(!core->cpmB) {
				while(cycCount <= cycPerFrame) {
					if(core->hltB)
						core->cycles += 4;
					else
						core->emulateCycle();

					if(core->opcode == 0xdb) { //game input
						switch(core->loc) {
							case 1: core->registers[0x7] = core->portIn[0]; break;
							case 2: core->registers[0x7] = core->portIn[1]; break; 
							case 3:	core->registers[0x7] = ((x << 8) | y) >> (8 - offset); break;
							default: cout << +core->loc << endl; break;
						}
					} else if(core->opcode == 0xd3) { //game output
						switch(core->loc) {
							case 2: offset = core->portOut[0] & 0x7; break;
							case 4: y = x; x = core->portOut[2]; break;
							case 3: 
								core->soundB = (core->portOut[1] & 0x20) != 0;
								// if(core->soundB) {
								// 	if((core->portOut[1] & 0x1) != 0)
								// 		Mix_PlayChannel(-1, wav1, 0);
								// 	if((core->portOut[1] & 0x2) != 0)
								// 		Mix_PlayChannel(-1, wav2, 0);
								// 	if((core->portOut[1] & 0x4) != 0)
								// 		Mix_PlayChannel(-1, wav3, 0);
								// 	if((core->portOut[1] & 0x8) != 0)
								// 		Mix_PlayChannel(-1, wav4, 0);
								// 	if((core->portOut[1] & 0x10) != 0)
								// 		Mix_PlayChannel(-1, wav5, 0);
								// }
							break;
							case 5:
								// if(core->soundB) {
								// 	if((core->portOut[3] & 0x1) != 0)
								// 		Mix_PlayChannel(-1, wav6, 0);
								// 	if((core->portOut[3] & 0x2) != 0)
								// 		Mix_PlayChannel(-1, wav7, 0);
								// 	if((core->portOut[3] & 0x4) != 0)
								// 		Mix_PlayChannel(-1, wav8, 0);
								// 	if((core->portOut[3] & 0x8) != 0)
								// 		Mix_PlayChannel(-1, wav9, 0);
								// 	if((core->portOut[3] & 0x10) != 0)
								// 		Mix_PlayChannel(-1, wav10, 0);
								// }
							break;

							case 6: break; //writes A to this port, debug thing?
							default: cout << +core->loc << endl; break;
						}
					}

					cycCount += core->cycles - core->cycBefore;
					if(core->cycles >= everyHalfCPF) {
						if(core->interruptB) {
							uint16_t pctmp = core->pc -= 3;
							uint8_t optmp = core->memory[pctmp];
							core->memory[pctmp] = ((vInterrupt) ? 0xcf : 0xd7);
							core->emulateCycle();
							core->memory[pctmp] = optmp;
							core->cycles -= 11;
						}
						core->cycles -= CLOCK/(fpsI*2);
						vInterrupt = !vInterrupt;
					}
				}
			}

			// SDL_UpdateTexture(texture, NULL, core->pixels, SCREEN_WIDTH * sizeof(uint8_t) * 3);
			// SDL_RenderCopy(renderer, texture, NULL, NULL);
			// SDL_RenderPresent(renderer);
		}

		if(core->cpmB) {		
			if(!core->hltB)
				core->emulateCycle();
		}
	}

	if(core->cpmB)
		fclose(core->cpmPrint);

	if(core->debugB)
		fclose(core->log); 

	// Mix_FreeChunk(wav1);
	// Mix_FreeChunk(wav2);
	// Mix_FreeChunk(wav3);
	// Mix_FreeChunk(wav4);
	// Mix_FreeChunk(wav5);
	// Mix_FreeChunk(wav6);
	// Mix_FreeChunk(wav7);
	// Mix_FreeChunk(wav8);
	// Mix_FreeChunk(wav9);
	// Mix_FreeChunk(wav10);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window); 
	SDL_Quit(); 
	return 0;   
}

void keyboard(bool press) {	
	const Uint8* state = SDL_GetKeyboardState(NULL);

	if(press) {
		if(state[SDL_SCANCODE_ESCAPE]) 	core->runB = false;
		if(state[SDL_SCANCODE_R]) 		SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
		if(state[SDL_SCANCODE_C]) 		core->portIn[0] |= 0b0001; 		//Coin Deposit :D
		if(state[SDL_SCANCODE_1]) 		core->portIn[0] |= 0b0100; 		//Player1 Start
		if(state[SDL_SCANCODE_2]) 		core->portIn[0] |= 0b0010; 		//Player2 Start
		if(state[SDL_SCANCODE_SPACE]) 	core->portIn[0] |= 0b00010000; 	//Player1 Shot
		if(state[SDL_SCANCODE_A]) 		core->portIn[0] |= 0b00100000; 	//Player1 Left
		if(state[SDL_SCANCODE_D]) 		core->portIn[0] |= 0b01000000; 	//Player1 Right
		if(state[SDL_SCANCODE_KP_0]) 	core->portIn[1] |= 0b00010000; 	//Player2 Shot
		if(state[SDL_SCANCODE_KP_4]) 	core->portIn[1] |= 0b00100000; 	//Player2 Left
		if(state[SDL_SCANCODE_KP_6]) 	core->portIn[1] |= 0b01000000; 	//Player2 Right
	} else {
		if(!state[SDL_SCANCODE_C]) 		core->portIn[0] &= 0b11111110; 	//clear bit0 in portIn[0]
		if(!state[SDL_SCANCODE_1]) 		core->portIn[0] &= 0b11111011; 	//clear bit2 in portIn[0]
		if(!state[SDL_SCANCODE_2]) 		core->portIn[0] &= 0b11111101; 	//clear bit1 in portIn[0]
		if(!state[SDL_SCANCODE_SPACE]) 	core->portIn[0] &= 0b11101111; 	//clear bit4 in portIn[0]
		if(!state[SDL_SCANCODE_A]) 		core->portIn[0] &= 0b11011111; 	//clear bit5 in portIn[0]
		if(!state[SDL_SCANCODE_D]) 		core->portIn[0] &= 0b10111111; 	//clear bit6 in portIn[0]
		if(!state[SDL_SCANCODE_KP_0]) 	core->portIn[1] &= 0b11101111; 	//clear bit4 in portIn[1]
		if(!state[SDL_SCANCODE_KP_4]) 	core->portIn[1] &= 0b11011111; 	//clear bit5 in portIn[1]
		if(!state[SDL_SCANCODE_KP_6])	core->portIn[1] &= 0b10111111; 	//clear bit6 in portIn[1]
	}
}