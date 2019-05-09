#include "si8080.h"

#define A 0x7
#define B 0x0
#define C 0x1
#define D 0x2
#define E 0x3
#define H 0x4
#define L 0x5

int cyclesTable[256] = {
//  0   1   2   3   4   5   6  7  8   9   a   b   c   d  e  f 
	4, 10,  7,  5,  5,  5,  7, 4, 4, 10,  7,  5,  5,  5, 7, 4,  //0
	4, 10,  7,  5,  5,  5,  7, 4, 4, 10,  7,  5,  5,  5, 7, 4,  //1
	4, 10, 16,  5,  5,  5,  7, 4, 4, 10, 16,  5,  5,  5, 7, 4,  //2
	4, 10, 13,  5,  5,  5, 10, 4, 4, 10, 13,  5,  5,  5, 7, 4,  //3
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //4
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //5
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //6
	7,  7,  7,  7,  7,  7,  7, 7, 5,  5,  5,  5,  5,  5, 7, 5, //7
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //8
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //9
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //a
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //b
	5, 10, 10, 10, 11, 11,  7, 0, 5,  4, 10,  4, 11, 11, 7, 0,  //c
	5, 10, 10, 10, 11, 11,  7, 0, 5,  4, 10, 10, 11,  4, 7, 0,  //d
	5, 10, 10, 18, 11, 11,  7, 0, 5,  5, 10,  4, 11,  4, 7, 0,  //e
	5, 10, 10,  4, 11, 11,  7, 0, 5,  5, 10,  4, 11,  4, 7, 0   //f
};

void si8080::emulateCycle() {
	opcode = memory[pc];
		
	loc = (registers[H] << 8) + registers[L];
	cycBefore = cycles;

	if(debug)
		fprintf(log, "PC:%04X OP:%02X ", pc, opcode);
	
	(this->*opcodes[opcode])();
	cycles += cyclesTable[opcode];

	if(debug)
		fprintf(log, "SP:%04X BC:%04X DE:%04X HL:%04X A:%02X Cy:%i AC:%i S:%i Z:%i P:%i\n",  sp, ((registers[B] << 8) + registers[C]), ((registers[D] << 8) + registers[E]), ((registers[H] << 8) + registers[L]), registers[A], cy, ac, s, z, p);
}

void si8080::nop() {}

void si8080::lxi() {
	uint8_t reg = (opcode >> 3) & 0x6;
	loc = (memory[pc+2] << 8) + memory[pc+1];
	if(reg == 0x6) {
		sp = loc;
	}
	else {
		registers[reg+1] = loc & 0xff;
		registers[reg] = (loc >> 8) & 0xff;
	}

	pc += 3;
} 

void si8080::stax() {
	uint8_t reg = (opcode >> 3) & 0x2;
	loc = (registers[reg] << 8) + registers[reg+1];
	changeM(registers[A]);
}

void si8080::inx() {
	uint8_t reg = (opcode >> 3) & 0x6;
 	if(reg == 0x6) {
		sp = (uint16_t)(sp + 1);
	}
	else {
		loc = ((registers[reg] << 8) + registers[reg+1]) + 1;
		registers[reg+1] = loc & 0xff;
		registers[reg] = (loc >> 8) & 0xff;
	}
}

void si8080::inr() {
	uint8_t reg = (opcode >> 3) & 0x7;
	loc = ((uint16_t) registers[H] << 8) + registers[L];

	if(reg == 0x6)
		changeM(setCond(memory[loc] + 1, memory[loc], 1));
	else
		registers[reg] = setCond(registers[reg] + 1, registers[reg], 1);
} 

void si8080::dcr() {
	uint8_t reg = (opcode >> 3) & 0x7;
	loc = ((uint16_t) registers[H] << 8) + registers[L];

	if(reg == 0x6)
		changeM(setCond((memory[loc] - 1) & 0xFF, memory[loc], 1)); 
	else
		registers[reg] = setCond((registers[reg] - 1) & 0xFF, registers[reg], 1);
} 

void si8080::mvi() {
	uint8_t reg = (opcode >> 3) & 0x7;
	loc = ((uint16_t) registers[H] << 8) + registers[L];
	if(reg == 0x6)
		changeM(memory[pc+1]);
	else 
		registers[reg] = memory[pc+1];

	pc += 1;
} 

void si8080::rlc() {
	cy = (((registers[A] & 0x80) == 0x80) ? 1 : 0);
	registers[A] = (registers[A] << 1) + cy; 
}

void si8080::dad() {
	uint8_t reg = (opcode >> 3) & 0x6;
	uint32_t tmp = loc;

	if(reg == 0x6) {
		tmp += sp;
	}
	else {
		tmp += (registers[reg] << 8) + registers[reg+1];
	}

	registers[L] = tmp & 0xff;
	registers[H] = (tmp >> 8) & 0xff;
	cy = (tmp > 0xffff);
} 

void si8080::ldax() {
	uint8_t reg = (opcode >> 3) & 0x2;

	loc = (registers[reg] << 8) + registers[reg+1];
	registers[A] = memory[loc];
}

void si8080::dcx() {
	uint8_t reg = (opcode >> 3) & 0x6;
	if(reg == 0x6) 
		sp = (uint16_t)(sp - 1);
	else {
		loc = ((registers[reg] << 8) + registers[reg+1]) - 1;
		registers[reg+1] = loc & 0xff;
		registers[reg] = (loc >> 8) & 0xff;
	}
}

void si8080::rrc() {
	cy = (((registers[A] & 0x1) == 0x1) ? 1 : 0);
	registers[A] = (registers[A] >> 1) + (cy << 7);
}

void si8080::ral() {
	registers[A] = (registers[A] << 1) + cy; 
	cy = (((registers[A] & 0x80) == 0x80) ? 1 : 0);
}

void si8080::rar() {
	registers[A] = (registers[A] >> 1) + (cy << 7); 
	cy = (((registers[A] & 0x1) == 0x1) ? 1 : 0);
}

void si8080::shld() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	changeM(registers[L]);
	loc += 1;
	changeM(registers[H]);
	pc += 2;
}

void si8080::daa() {
    uint8_t lsb = registers[A] & 0xf;
    uint8_t msb = registers[A] >> 4;

    if (ac || lsb > 9) {
        registers[A] = setCond(registers[A] + 0x6, registers[A], 0x6);
    }
    if (cy || msb > 9) {
        registers[A] = setCond(registers[A] + 0x60, registers[A], 0x60);
    }
}

void si8080::lhld() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	registers[L] = memory[loc];
	registers[H] = memory[loc+1];

	pc += 2;
}

void si8080::cma() {
	registers[A] = (registers[A] == 1) ? 0 : 1;
}

void si8080::sta() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	changeM(registers[A]);

	pc += 2;
}

void si8080::stc() {
	cy = 1;
}

void si8080::lda() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	registers[A] = memory[loc];
	
	pc += 2;
}

void si8080::cmc() {
	cy = (cy == 1) ? 0 : 1;
}

void si8080::mov() {
	uint8_t dst = (opcode >> 3) & 0x7;
	uint8_t src = opcode & 0x7;

	if(dst == 0x6)
		changeM(registers[src]);
	else if(src == 0x6)
		registers[dst] = memory[loc];
	else
		registers[dst] = registers[src];
}

void si8080::hlt() {
	hltB = true;
}

void si8080::math() {
	uint8_t data = registers[opcode & 0x7];

	if((opcode & 0xc0) == 0xc0) {
		data = memory[pc+1];
		pc += 1;
	} else if((opcode & 0x7) == 0x6)
		data = memory[loc];

	switch((opcode >> 3) & 0x7) {
		case 0x7: setCond(registers[A] - data, registers[A], data); break;
		case 0x0: registers[A] = setCond(registers[A] + data, registers[A], data); break;
		case 0x1: registers[A] = setCond(registers[A] + (data + cy), registers[A], (data + cy)); break;
		case 0x2: registers[A] = setCond(registers[A] - data, registers[A], data); break;
		case 0x3: registers[A] = setCond(registers[A] - (data - cy), registers[A], (data + cy)); break;
		case 0x4: registers[A] = setCond(registers[A] & data, 0, 0);
				  ac = ((registers[A] | data) & 0x8) != 0;
				  cy = 0;
		break;
		case 0x5: registers[A] = setCond(registers[A] ^ data, 0, 0); 
				  ac = cy = 0;
		break;
		case 0x6: registers[A] = setCond(registers[A] | data, 0, 0); 
				  ac = cy = 0;
		break; 
	}
}

void si8080::retC() { //11 cond(3) 000
	if(checkCond())
		ret();
}

void si8080::pop() { //11 rp(2) 0001
	uint8_t reg = (opcode >> 3) & 0x6;

	if(reg == 0x6) {
		uint8_t flags = memory[sp];
		cy = flags & 0x1;
		ac = (flags & 0x10) >> 4;
		s  = (flags & 0x80) >> 7;
		z  = (flags & 0x40) >> 6;
		p  = (flags & 0x4) >> 2;
		registers[A] = memory[sp+1];
	} else {
		registers[reg+1] = memory[sp];
		registers[reg] = memory[sp+1];
	}

	sp += 2;
}

void si8080::jmpC() { //11 cond(3) 010
	if(checkCond()) 
		jmp();
	else
		pc += 3;
}

void si8080::jmp() {
	pc = (memory[pc+2] << 8) + memory[pc+1];
}

void si8080::callC() { //11 cond(3) 100
	if(checkCond())
		call();
	else
		pc += 3;
}

void si8080::push() { //11 rp(2) 0101
	uint8_t reg = (opcode >> 3) & 0x6;

	if(reg == 0x6) { //PSW
		memory[sp-2] = (s << 7) + (z << 6) + (ac << 4) + (p << 2) + 0x2 + cy;
		memory[sp-1] = registers[A];
	} else {
		memory[sp-2] = registers[reg+1];
		memory[sp-1] = registers[reg];
	}

	sp -= 2;
}

void si8080::rst() { //11 exp(3) 111
	call();
	pc = opcode & 0x38;

	cycles += 5;
	hltB = false;
	interrupt = false;
}

void si8080::ret() {
	pc = (memory[sp+1] << 8) + memory[sp];
	sp += 2;
	cycles += 6;
}

void si8080::call() {
	memory[sp-2] = (pc + 3) & 0xff;
	memory[sp-1] = (pc + 3) >> 8;
	sp -= 2;
	pc = (memory[pc+2] << 8) + memory[pc+1];
	cycles += 6;
}

void si8080::out() {
	loc = memory[pc+1];
	portOut[loc-2] = registers[A]; //2->0
	pc += 2;
}

void si8080::in() {
	loc = memory[pc+1];
	registers[A] = portIn[loc-1]; //1->0
	pc += 2;
}

void si8080::xthl() {
	uint8_t tmp2 = memory[sp];
	uint8_t tmp1 = memory[sp+1];

	//replace 2 bytes on stack with H+L
	loc = sp;
	changeM(registers[L]);
	loc++;
	changeM(registers[H]);

	//replace H+L with 2 bytes from stack
	registers[L] = tmp2;
	registers[H] = tmp1;
}

void si8080::xchg() {
	uint8_t tmp2 = registers[E];
	uint8_t tmp1 = registers[D];
	registers[E] = registers[L];
	registers[D] = registers[H];
	registers[L] = tmp2;
	registers[H] = tmp1;
}

void si8080::di() {
	interrupt = false;
}

void si8080::pchl() {
	pc = loc;
}

void si8080::sphl() {
	sp = loc;
}

void si8080::ei() {
	interrupt = true;
}

void si8080::changeM(uint8_t value) { //it was getting in the way tbh
	if(loc >= 0x2400 && loc < 0x4000) {
		int offset = (loc - 0x2400) * 8; //send this and the value over and copy the rest of the code below
		int x = offset / 256;			 //and update the screen every 1/60th of a second (but don't send the
		int y = 255 - (offset % 256);	 //pixels at that framerate)
		
		for(int i = 0; i < 8; i++) {
			bool bit = (((value >> i) & 0x1) == 0x1);
			int location = ((y * 224) - (i * 224) + x) * 3;
			
			if(bit) {
				if(y > 240) {
					if(x < 16) {						//white 
						pixels[location] = 0xFF; 	//R
						pixels[location+1] = 0xFF;	//G
						pixels[location+2] = 0xFF;	//B
					} else if(x < 118) {				//green 
						pixels[location] = 0x0; 	//R
						pixels[location+1] = 0xFF;	//G
						pixels[location+2] = 0x0;	//B
					} else if(x < 224) {				//white 
						pixels[location] = 0xFF; 	//R
						pixels[location+1] = 0xFF;	//G
						pixels[location+2] = 0xFF;	//B
					}
				} else if(y > 184) {					//green 
						pixels[location] = 0x0; 	//R
						pixels[location+1] = 0xFF;	//G
						pixels[location+2] = 0x0;	//B
				} else if(y > 48) {						//white 
						pixels[location] = 0xFF; 	//R
						pixels[location+1] = 0xFF;	//G
						pixels[location+2] = 0xFF;	//B
				} else if(y > 32) {						//red 
						pixels[location] = 0xFF; 	//R
						pixels[location+1] = 0x0;	//G
						pixels[location+2] = 0x0;	//B
				} else if(y > 0) {						//white 
						pixels[location] = 0xFF; 	//R
						pixels[location+1] = 0xFF;	//G
						pixels[location+2] = 0xFF;	//B
				}
			} else { //black
				pixels[location] = 0x0;
				pixels[location+1] = 0x0;
				pixels[location+2] = 0x0;
			}
		}
	}
	memory[loc] = value;
} 

bool si8080::checkCond() {
	uint8_t check = (opcode >> 3) & 0x7;

	if((check == 0x0 && !z) || (check == 0x1 && z) || (check == 0x2 && !cy) || (check == 0x3 && cy) || (check == 0x4 && !p) || (check == 0x5 && p) || (check == 0x6 && !s) || (check == 0x7 && s))
		return true;

	return false;
}

uint8_t si8080::setCond(uint16_t ans, uint8_t old, uint8_t diff) {
	cy = (ans > 0xff);
	ac = (((old ^ diff ^ ans) & 0x10) == 0x10);
	s = ((ans & 0x80) == 0x80);
	z = ((ans & 0xff) == 0);
	p = !__builtin_parity(ans & 0xff);

	return (ans & 0xff);
}

void si8080::load(const char* filename) {
	FILE* rom = fopen(filename, "rb");
	if (rom == NULL) {
		fputs("File error", stderr); 
	} else {
		fseek(rom, 0, SEEK_END);
		romSize = ftell(rom);
		rewind(rom);
		
		char* buffer = (char*)malloc(sizeof(char) * romSize);
		if (buffer == NULL) {
			fputs("Memory error", stderr);
		} else {
			uint16_t result = fread(buffer, 1, romSize, rom);
			if (result != romSize) {
				fputs("Reading error", stderr); 
			} else {
				uint32_t tmp = romSize + 0x2000;
				for(uint32_t i = 0; i < tmp; i++)
					memory.push_back(0);	

				pc = sp = cycles = cycBefore = 0;
				interrupt = hltB = false;
				debug = false;

				fclose(rom);
				free(buffer);
			}
		}
	}
	
	//do space invaders stuff
	pixels = new uint8_t[224 * 256 * 3]; //vram is 224*32 bytes, each bit is a pixel

	for(int x = 0; x < 224; x++) { 
		for(int y = 0; y < 256; y++) { //set each pixel to black
			loc = x + (y * 224);
			pixels[loc] 	= 0x0;
			pixels[loc + 1] = 0x0;
			pixels[loc + 2] = 0x0;
		}
	}

	for(int i = 0; i <= 8; i++)
		registers[i] = 0;

	//each word is a bit starting from bit 7 (x = nothing, ? = not sure, 1 = always on)
	portIn[0] =  0b1000; //x 1right 1left 1fire 1 1p 2p credit
	portIn[1] =  0b1000; //sw1 2right 2left 2fire sw2 tilt(0) sw3 sw4 http://www.brentradio.com/images/Other/Docs/SpaceInvaders/dipswitchsettings.txt
	portIn[2] =  0b0; //shift register
	portOut[0] = 0b0; //00000bbb shift amount
	portOut[1] = 0b0; //x x amp extended sound_aliendeath sound_playerdeath sound_shot sound_ufo
	portOut[2] = 0b0; //shift data
	portOut[3] = 0b0; //x x x sound_ufodeath sound_fleet4 sound_fleet3 sound_fleet2 sound_fleet1
	portOut[4] = 0b0; //watchdog

	if(debug) {
		log = fopen("log.txt", "w");
		fprintf(log, "Loaded: %s\nSize: %u\n", filename, romSize);
	}
}