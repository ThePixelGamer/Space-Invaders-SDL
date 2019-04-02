#pragma once

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <bitset>

#include "SDL2/SDL.h"

using namespace std;

class si8080 {
    uint8_t         registers[8];          //b, c, d, e, h, l, not using, a
    uint8_t         cy, ac, s, z, p;       //conditions

	uint16_t        pc, sp;                //counter + stack pointer
	uint8_t         memory[0x4000 * 2];    //4kb + a mirror of it ¯\_(ツ)_/¯
    long            romSize;
    int             vramStart;
    int             vramEnd;
    uint16_t        loc;

public:
    bool            debug;

	bool			drawFlag;
	uint32_t*		pixels;      //duplicate of vram but in 32 rgba format
    uint8_t			port[9];     //read 0, read 1, read 2, read 3, write 2, write 3, write 4, write 5, write 6
	
    int       		cycles;
    int       		cycBefore;
    int       		cycCount;
    bool            interrupt;	
	
	si8080();

	void            emulateCycle(uint8_t);
	uint8_t         checkCond(uint8_t);
	uint8_t         setCond8(uint16_t, uint8_t, uint8_t, uint8_t);
	uint8_t         checkParity(uint8_t);
	uint8_t         checkAC(uint8_t, uint16_t, uint16_t);
	void            changeM(uint8_t);    //probably a really dumb way to do it
    string          load(const char*);

    //0x00 - 0x3f
    void            lxi(uint8_t);             
    void            dad(uint8_t);              
    void            stax(uint8_t);           
    void            ldax(uint8_t);             
    void            inx(uint8_t);           
    void            dcx(uint8_t);           
    void            inr(uint8_t);             
    void            dcr(uint8_t);             
    void            mvi(uint8_t);            
    //0x40 - 0x7f
    void            mov(uint8_t, uint8_t);     
    //0x80 - 0xbf
    void            math(); //get's reused in the c0 section with 0x06 and 0x0e instructions 
    //0xc0 - 0xff       
    void            retC(uint8_t); //return condition
    void            pop(uint8_t); 
    void            jmpC(uint8_t); //jump condition            
    void            callC(uint8_t); //call condition
    void            push(uint8_t);              
    void            rst(uint8_t); 
    void            jmp();  
    void            ret();
    void            call()         
};  