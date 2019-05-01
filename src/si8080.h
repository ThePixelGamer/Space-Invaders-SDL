#pragma once

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "SDL2/SDL.h"

using namespace std;

class si8080 {
public:
	vector<uint8_t> memory;             //4kb
	uint16_t        pc, sp, loc;        //counter + stack pointer
    uint8_t         registers[8];       //b, c, d, e, h, l, not using, a
    uint8_t         opcode, cy, ac, z, p, s, interrupt, hlt, cpmB, debug;
    uint32_t        romSize, vramStart, cycles, cycBefore;

    FILE*           log;
	uint8_t*		pixels;       //duplicate of vram but in 24 rgb format
    uint8_t		    portOut[5];   //write 2, write 3, write 4, write 5, write 6
    uint16_t		portIn[3];    //read 1, read 2, read 3

	void            emulateCycle();
	bool            checkCond();
	uint8_t         setCond(uint16_t, uint8_t, uint8_t);
	void            changeM(uint8_t);  
    void            load(const char*);

    void            cpm(); //cp/m implementation
    void            lxi(); //0x00 - 0x3f
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
    void            mov();   //0x40 - 0x7f 
    void            math(); //0x80 - 0xbf  
    void            retC(); //0xc0 - 0xff
    void            pop();
    void            jmpC();  
    void            jmp();        
    void            callC(); 
    void            push();
    void            rst();
    void            ret();
    void            call();
    void            out();
    void            in();
    void            xthl();
    void            xchg();
};