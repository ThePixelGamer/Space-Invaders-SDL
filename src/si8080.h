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
    uint8_t         opcode, registers[8], cy, ac, z, p, s, interruptB, hltB, soundB, cpmB, debugB, portOut[5], portIn[3];
    uint16_t        pc, sp, loc, cycles, cycBefore;
    uint32_t        romSize, vramStart;

    FILE*           log;
    uint8_t*        pixels; //duplicate of vram but in 24 rgb format

    void            emulateCycle();
    bool            checkCond();
    uint8_t         setCond(uint16_t, uint8_t, uint8_t);
    void            changeM(uint8_t);
    void            load(const char*);

    void            end();
    void            cpm();

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
    
    static constexpr void (si8080::*opcodeTable[256])() = {
    //             0              1               2             3              4               5              6              7              8              9               a             b              c               d              e              f 
        &si8080::nop,  &si8080::lxi,  &si8080::stax, &si8080::inx,  &si8080::inr,   &si8080::dcr,  &si8080::mvi,  &si8080::rlc,  &si8080::nop,  &si8080::dad,  &si8080::ldax, &si8080::dcx,  &si8080::inr,   &si8080::dcr,  &si8080::mvi,  &si8080::rrc,  //0
        &si8080::end,  &si8080::lxi,  &si8080::stax, &si8080::inx,  &si8080::inr,   &si8080::dcr,  &si8080::mvi,  &si8080::ral,  &si8080::nop,  &si8080::dad,  &si8080::ldax, &si8080::dcx,  &si8080::inr,   &si8080::dcr,  &si8080::mvi,  &si8080::rar,  //1
        &si8080::cpm,  &si8080::lxi,  &si8080::shld, &si8080::inx,  &si8080::inr,   &si8080::dcr,  &si8080::mvi,  &si8080::daa,  &si8080::nop,  &si8080::dad,  &si8080::lhld, &si8080::dcx,  &si8080::inr,   &si8080::dcr,  &si8080::mvi,  &si8080::cma,  //2
        &si8080::nop,  &si8080::lxi,  &si8080::sta,  &si8080::inx,  &si8080::inr,   &si8080::dcr,  &si8080::mvi,  &si8080::stc,  &si8080::nop,  &si8080::dad,  &si8080::lda,  &si8080::dcx,  &si8080::inr,   &si8080::dcr,  &si8080::mvi,  &si8080::cmc,  //3
        &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,   &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,   &si8080::mov,  &si8080::mov,  &si8080::mov,  //4
        &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,   &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,   &si8080::mov,  &si8080::mov,  &si8080::mov,  //5
        &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,   &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,   &si8080::mov,  &si8080::mov,  &si8080::mov,  //6
        &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,   &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,  &si8080::mov,   &si8080::mov,  &si8080::mov,  &si8080::mov,  //7
        &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math,  &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math,  &si8080::math, &si8080::math, &si8080::math, //8
        &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math,  &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math,  &si8080::math, &si8080::math, &si8080::math, //9
        &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math,  &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math,  &si8080::math, &si8080::math, &si8080::math, //a
        &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math,  &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math, &si8080::math,  &si8080::math, &si8080::math, &si8080::math, //b
        &si8080::retC, &si8080::pop,  &si8080::jmpC, &si8080::jmp,  &si8080::callC, &si8080::push, &si8080::math, &si8080::rst,  &si8080::retC, &si8080::ret,  &si8080::jmpC, &si8080::nop,  &si8080::callC, &si8080::call, &si8080::math, &si8080::rst,  //c
        &si8080::retC, &si8080::pop,  &si8080::jmpC, &si8080::out,  &si8080::callC, &si8080::push, &si8080::math, &si8080::rst,  &si8080::retC, &si8080::nop,  &si8080::jmpC, &si8080::in,   &si8080::callC, &si8080::nop,  &si8080::math, &si8080::rst,  //d
        &si8080::retC, &si8080::pop,  &si8080::jmpC, &si8080::xthl, &si8080::callC, &si8080::push, &si8080::math, &si8080::rst,  &si8080::retC, &si8080::pchl, &si8080::jmpC, &si8080::xchg, &si8080::callC, &si8080::nop,  &si8080::math, &si8080::rst,  //e
        &si8080::retC, &si8080::pop,  &si8080::jmpC, &si8080::di,   &si8080::callC, &si8080::push, &si8080::math, &si8080::rst,  &si8080::retC, &si8080::sphl, &si8080::jmpC, &si8080::ei,   &si8080::callC, &si8080::nop,  &si8080::math, &si8080::rst   //f
    }; 
};