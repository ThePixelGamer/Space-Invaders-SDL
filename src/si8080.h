#pragma once

#include "SDL2/SDL.h"

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

class si8080 {
public:

    vector<uint8_t> memory;
    uint8_t         opcode, cy, ac, z, p, s, interrupt, hltB, debug, registers[8];
    uint16_t        pc, sp, loc;
    uint32_t        romSize, vramStart, cycles, cycBefore;

    FILE*           log;
    uint8_t*        pixels; //duplicate of vram but in 24 rgb format
    uint8_t         portOut[5], portIn[3];

    void            emulateCycle();
    bool            checkCond();
    uint8_t         setCond(uint16_t, uint8_t, uint8_t);
    void            changeM(uint8_t);  
    void            load(const char*);

    void            nop();  //0x00 - 0x3f
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
    void            mov();  //0x40 - 0x7f
    void            hlt(); 
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
    void            di();
    void            pchl();
    void            sphl();
    void            ei();
    
    static constexpr void (si8080::*opcodes[256])() = {
    //	   0     1     2     3      4     5     6     7     8     9      a    b      c     d     e     f 
        nop,  lxi, stax,  inx,   inr,  dcr,  mvi,  rlc,  nop,  dad,  ldax, dcx,   inr,  dcr,  mvi,  rrc,  //0
        nop,  lxi, stax,  inx,   inr,  dcr,  mvi,  ral,  nop,  dad,  ldax, dcx,   inr,  dcr,  mvi,  rar,  //1
        nop,  lxi, shld,  inx,   inr,  dcr,  mvi,  daa,  nop,  dad,  lhld, dcx,   inr,  dcr,  mvi,  cma,  //2
        nop,  lxi,  sta,  inx,   inr,  dcr,  mvi,  stc,  nop,  dad,  lda,  dcx,   inr,  dcr,  mvi,  cmc,  //3
        mov,  mov,  mov,  mov,   mov,  mov,  mov,  mov,  mov,  mov,  mov,  mov,   mov,  mov,  mov,  mov,  //4
        mov,  mov,  mov,  mov,   mov,  mov,  mov,  mov,  mov,  mov,  mov,  mov,   mov,  mov,  mov,  mov,  //5
        mov,  mov,  mov,  mov,   mov,  mov,  mov,  mov,  mov,  mov,  mov,  mov,   mov,  mov,  mov,  mov,  //6
        mov,  mov,  mov,  mov,   mov,  mov,  mov,  mov,  mov,  mov,  mov,  mov,   mov,  mov,  mov,  mov,  //7
        math, math, math, math,  math, math, math, math, math, math, math, math,  math, math, math, math, //8
        math, math, math, math,  math, math, math, math, math, math, math, math,  math, math, math, math, //9
        math, math, math, math,  math, math, math, math, math, math, math, math,  math, math, math, math, //a
        math, math, math, math,  math, math, math, math, math, math, math, math,  math, math, math, math, //b
        retC,  pop, jmpC,  jmp, callC, push, math,  rst, retC,  ret, jmpC,  nop, callC, call, math,  rst,  //c
        retC,  pop, jmpC,  out, callC, push, math,  rst, retC,  nop, jmpC,   in, callC,  nop, math,  rst,  //d
        retC,  pop, jmpC, xthl, callC, push, math,  rst, retC, pchl, jmpC, xchg, callC,  nop, math,  rst,  //e
        retC,  pop, jmpC,   di, callC, push, math,  rst, retC, sphl, jmpC,   ei, callC,  nop, math,  rst   //f
    };
};