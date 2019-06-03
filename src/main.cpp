#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_mixer.h"

//needs iostream or some parts but it's already included from si8080 so... :P
#include <chrono>
#include <cmath>
#include <conio.h>
#include <windows.h>

#include "si8080.h"

using namespace chrono; 
using frame = duration<int32_t, ratio<1, 60>>;

constexpr auto SCREEN_HEIGHT = 256;
constexpr auto SCREEN_WIDTH = 224;
constexpr auto CLOCK = 2000000;

SDL_Window*		window;
SDL_Renderer*	renderer;
SDL_Texture*	texture;
SDL_Event		event;
SDL_DisplayMode dm;

si8080* core = new si8080();
bool vInterrupt = true;
uint8_t port1 = 0b1000, port2 = 0b1000, x, y, offset;
uint32_t cycCount = 0, fpsI = 60;
const Uint8* state;

int main(int argc, char* args[]) {
	if(argc > 1) {
		AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);

		core->cpmB = true;
		core->load(args[1]);

		while(core->runB)
			core->emulateCycle();

		printf("\n\nPress any key to continue...");
		getch();

		fclose(core->cpmPrint);
		fclose(core->log);
		FreeConsole();
	} else {
		core->cpmB = false;
		core->load("invaders.com");

		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		SDL_GetCurrentDisplayMode(0, &dm);

		window = SDL_CreateWindow("Space Invaders", (dm.w/2)-(SCREEN_WIDTH/2), (dm.h/2)-(SCREEN_HEIGHT/2), SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SDL_WINDOW_RESIZABLE);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
		double cycPerFrame = round((double)CLOCK / fpsI);
		double everyHalfCPF = round((double)CLOCK / (fpsI * 2));

		time_point<steady_clock> fpsTimer(steady_clock::now());
		frame fps{};
		while(core->runB) {
			fps = duration_cast<frame>(steady_clock::now() - fpsTimer);
			if(fps.count() >= 1) {
				fpsTimer = steady_clock::now();
				cycCount = 0;

				while(SDL_PollEvent(&event)) { 
					switch(event.type) {
						case SDL_QUIT: core->runB = false; break;
					}
				}

				state = SDL_GetKeyboardState(NULL);

				core->portIn[0] &= 0b10001000; //thanks milkdud :P
				core->portIn[1] &= 0b10001111;

				if(state[SDL_SCANCODE_ESCAPE]) 	core->runB = false;
				if(state[SDL_SCANCODE_R]) 		SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
				if(state[SDL_SCANCODE_C]) 		core->portIn[0] |= 0b00000001; //Coin Deposit :D
				if(state[SDL_SCANCODE_1]) 		core->portIn[0] |= 0b00000100; //Player1 Start
				if(state[SDL_SCANCODE_2]) 		core->portIn[0] |= 0b00000010; //Player2 Start
				if(state[SDL_SCANCODE_SPACE]) 	core->portIn[0] |= 0b00010000; //Player1 Shot
				if(state[SDL_SCANCODE_A]) 		core->portIn[0] |= 0b00100000; //Player1 Left
				if(state[SDL_SCANCODE_D]) 		core->portIn[0] |= 0b01000000; //Player1 Right
				if(state[SDL_SCANCODE_KP_0]) 	core->portIn[1] |= 0b00010000; //Player2 Shot
				if(state[SDL_SCANCODE_KP_4]) 	core->portIn[1] |= 0b00100000; //Player2 Left
				if(state[SDL_SCANCODE_KP_6]) 	core->portIn[1] |= 0b01000000; //Player2 Right
				
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
							case 3: core->soundB = (core->portOut[1] & 0x20) != 0; break; //sound shit
							case 5: break; //sound shit
							case 6: break; //debug thing
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
						}
						core->cycles -= everyHalfCPF;
						vInterrupt = !vInterrupt;
					}
				}

				SDL_UpdateTexture(texture, NULL, core->pixels, SCREEN_WIDTH * sizeof(uint8_t) * 3);
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				SDL_RenderPresent(renderer);
			}
		}
			
		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window); 
		SDL_Quit(); 
	}	

	return 0;   
}