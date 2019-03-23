#pragma once

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "SDL2/SDL.h"

class si8080 {
public:
	bool			autoOpcode = true;
	bool			drawFlag;
	Uint32*			pixels;
    char[5]			port; //read 1, read 2, read 3, write 2, write 4
	
	si8080();

	void init();
	std::string emulateCycle();
	bool load(const char* filename);

//fuck your privacy
    unsigned char  a;                   //accumulator
    unsigned char  b;                   //register pair for b + c
    unsigned char  c;
    unsigned char  d;                   //register pair for d + e
    unsigned char  e;
    unsigned char  h;                   //register pair for h + l
    unsigned char  l;

    unsigned char  c;                   //carry
    unsigned char  ac;                  //auiliary carry
    unsigned char  s;                   //sign
    unsigned char  z;                   //zero
    unsigned char  p;                   //parity

	unsigned short pc;				    // program counter
	unsigned short sp;				    // Stack pointer
	
	unsigned char  memory[4096 * 2];    //4kb + a mirror of it ¯\_(ツ)_/¯		
};