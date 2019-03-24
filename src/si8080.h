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
	bool			autoOpcode = true;
	bool			drawFlag;
	uint32_t*			pixels;
    uint8_t			port[5]; //read 1, read 2, read 3, write 2, write 4
	
	si8080();

	void emulateCycle();
	uint8_t checkCond(uint16_t);
	uint8_t checkParity(uint16_t);
	bool load(const char* filename);

private:
    uint8_t  a;                   //accumulator
    uint8_t  b;                   //register pair for b + c
    uint8_t  c;
    uint8_t  d;                   //register pair for d + e
    uint8_t  e;
    uint8_t  h;                   //register pair for h + l
    uint8_t  l;

    uint8_t  cy;                   //carry
    uint8_t  ac;                  //auiliary carry
    uint8_t  s;                   //sign
    uint8_t  z;                   //zero
    uint8_t  p;                   //parity

	uint16_t pc;				    // program counter
	uint16_t sp;				    // Stack pointer
	
	uint8_t  *memory[4096 * 2];    //4kb + a mirror of it ¯\_(ツ)_/¯		
};