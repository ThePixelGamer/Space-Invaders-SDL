#pragma once

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <bitset>
#include <vector>

#include "SDL2/SDL.h"

using namespace std;

class si8080 {
public:
    uint8_t         registers[8];         //b, c, d, e, h, l, not using, a
    uint8_t         cy, ac, s, z, p;      //conditions

	uint16_t        pc, sp, loc;//counter + stack pointer
	vector<uint8_t> memory;       //4kb
    long            romSize;
    int             vramStart;

    FILE*           log;

	uint32_t*		pixels;     //duplicate of vram but in 32 rgba format
    uint8_t			port[9];    //read 0, read 1, read 2, read 3, write 2, write 3, write 4, write 5, write 6
	
    int       		cycles, cycBefore;
    bool            interrupt, hlt, drawFlag, debug, cmp;
    uint8_t         opcode;

	void            emulateCycle();
	bool            checkCond();
	uint8_t         setCond(uint16_t, uint8_t, uint8_t, uint8_t);
	uint8_t         checkParity(uint8_t);
    uint8_t         checkAC(uint16_t, uint8_t, uint8_t);
	void            changeM(uint8_t);   //probably a really dumb way to do it
    void            load(const char*);

    void            cpm();

    //0x00 - 0x3f
    void            lxi();
    void            stax();
    void            inx();
    void            inr();
    void            dcr();
    void            mvi();
    void            rlc();
    void            dad();
    void            ldax();
    void            dcx();
    void            rrc();
    void            ral();
    void            rar();
    void            shld();
    void            daa();
    void            lhld();
    void            cma();
    void            sta();
    void            stc();
    void            lda();
    void            cmc();        
    //0x40 - 0x7f
    void            mov();    
    //0x80 - 0xbf
    void            math(); //get's reused in the c0 section with 0x06 and 0x0e instructions 
    //0xc0 - 0xff        
    void            retC(); //return condition
    void            pop();
    void            jmpC(); //jump condition    
    void            jmp();        
    void            callC(); //call condition
    void            push();
    void            rst();
    void            ret();
    void            call();
    void            out();
    void            in();
    void            xthl();
    void            xchg();
};