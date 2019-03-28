#pragma once

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <bitset>

#include "SDL2/SDL.h"

using namespace std;

class si8080 {
public:
	bool			drawFlag;
	uint32_t*		pixels;      //duplicate of vram but in 32 rgba format
    uint8_t			port[5];                //read 1, read 2, read 3, write 2, write 4
	
	si8080();

	void            emulateCycle();
	uint16_t        setCond(uint16_t, uint16_t, uint16_t, uint8_t);
	uint8_t         checkParity(uint8_t);
	uint8_t         checkAC(uint8_t, uint16_t, uint16_t);
	void            vramChange(uint8_t);    //probably a really dumb way to do it

private:
    uint8_t         a, b, c, d, e, h, l;   //registers
    uint8_t         cy, ac, s, z, p;       //conditions

	uint16_t        pc, sp;                //counter + stack pointer
	uint8_t         memory[4096 * 2];      //4kb + a mirror of it ¯\_(ツ)_/¯		
};  