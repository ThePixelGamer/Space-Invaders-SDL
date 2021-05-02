#include "si8080.h"

constexpr auto A = 0x7;
constexpr auto B = 0x0;
constexpr auto C = 0x1;
constexpr auto D = 0x2;
constexpr auto E = 0x3;
constexpr auto H = 0x4;
constexpr auto L = 0x5;

constexpr void (si8080::*si8080::opcodeTable[256])();
int cyclesTable[256] = {
//  0   1   2   3   4   5   6  7  8   9   a   b   c   d  e  f 
	4, 10,  7,  5,  5,  5,  7, 4, 4, 10,  7,  5,  5,  5, 7, 4, //0
	4, 10,  7,  5,  5,  5,  7, 4, 4, 10,  7,  5,  5,  5, 7, 4, //1
	4, 10, 16,  5,  5,  5,  7, 4, 4, 10, 16,  5,  5,  5, 7, 4, //2
	4, 10, 13,  5,  5,  5, 10, 4, 4, 10, 13,  5,  5,  5, 7, 4, //3
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //4
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //5
	5,  5,  5,  5,  5,  5,  7, 5, 5,  5,  5,  5,  5,  5, 7, 5, //6
	7,  7,  7,  7,  7,  7,  7, 7, 5,  5,  5,  5,  5,  5, 7, 5, //7
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //8
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //9
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //a
	4,  4,  4,  4,  4,  4,  7, 4, 4,  4,  4,  4,  4,  4, 7, 4, //b
	5, 10, 10, 10, 11, 11,  7, 5, 5,  4, 10,  4, 11, 11, 7, 5, //c
	5, 10, 10, 10, 11, 11,  7, 5, 5,  4, 10, 10, 11,  4, 7, 5, //d
	5, 10, 10, 18, 11, 11,  7, 5, 5,  5, 10,  4, 11,  4, 7, 5, //e
	5, 10, 10,  4, 11, 11,  7, 5, 5,  5, 10,  4, 11,  4, 7, 5  //f
};
int pcTable[256] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f 
	1, 3, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, 1, 1, 2, 1, //0
	0, 3, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, 1, 1, 2, 1, //1
	0, 3, 3, 1, 1, 1, 2, 1, 0, 1, 3, 1, 1, 1, 2, 1, //2
	0, 3, 3, 1, 1, 1, 2, 1, 0, 1, 3, 1, 1, 1, 2, 1, //3
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //4
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //5
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //6
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //7
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //8
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //9
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //a
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //b
	0, 1, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, //c
	0, 1, 0, 2, 0, 1, 2, 0, 0, 0, 0, 2, 0, 0, 2, 0, //d
	0, 1, 0, 1, 0, 1, 2, 0, 0, 0, 0, 1, 0, 0, 2, 0, //e
	0, 1, 0, 1, 0, 1, 2, 0, 0, 1, 0, 1, 0, 0, 2, 0  //f
};

void si8080::emulateCycle() {
	opcode = memory[pc];
		
	loc = (registers[H] << 8) + registers[L];
	cycBefore = cycles;

	if(debugB)
		fprintf(log, "PC:%04x OP:%02x SP:%04x BC:%04x DE:%04x HL:%04x A:%02x Cy:%i AC:%i S:%i Z:%i P:%i\n", pc, opcode, sp, ((registers[B] << 8) + registers[C]), ((registers[D] << 8) + registers[E]), loc, registers[A], cy, ac, s, z, p);

	(this->*opcodeTable[opcode])();
	cycles += cyclesTable[opcode];
	pc += pcTable[opcode];
}

void si8080::nop() {}

void si8080::lxi() {
	uint8_t reg = (opcode >> 3) & 0x6;
	loc = (memory[pc+2] << 8) + memory[pc+1];
	if(reg == 0x6)
		sp = loc;
	else {
		registers[reg+1] = loc & 0xff;
		registers[reg] = loc >> 8;
	}
} 

void si8080::stax() {
	uint8_t reg = (opcode >> 3) & 0x2;
	loc = (registers[reg] << 8) + registers[reg+1];
	changeM(registers[A]);
}

void si8080::ldax() {
	uint8_t reg = (opcode >> 3) & 0x2;
	loc = (registers[reg] << 8) + registers[reg+1];
	registers[A] = memory[loc];
}

void si8080::inx() {
	uint8_t reg = (opcode >> 3) & 0x6;
	
 	if(reg == 0x6) 
		sp += 1;
	else {
		loc = ((registers[reg] << 8) + registers[reg+1]) + 1;
		registers[reg+1] = loc & 0xff;
		registers[reg] = loc >> 8;
	}
}

void si8080::dcx() {
	uint8_t reg = (opcode >> 3) & 0x6;

	if(reg == 0x6) 
		sp -= 1;
	else {
		loc = ((registers[reg] << 8) + registers[reg+1]) - 1;
		registers[reg+1] = loc & 0xff;
		registers[reg] = loc >> 8;
	}
}

void si8080::inr() {
	uint8_t reg = (opcode >> 3) & 0x7;
	bool c = cy;

	if(reg == 0x6)
		changeM(setCond(memory[loc] + 1, memory[loc], 1));
	else
		registers[reg] = setCond(registers[reg] + 1, registers[reg], 1);
		
	cy = c;
} 

void si8080::dcr() {
	uint8_t reg = (opcode >> 3) & 0x7;
	bool c = cy;

	if(reg == 0x6)
		changeM(setCond(memory[loc] - 1, memory[loc], (~1 + 1))); 
	else
		registers[reg] = setCond(registers[reg] - 1, registers[reg], (~1 + 1));

	cy = c;
} 

void si8080::dad() {
	uint8_t reg = (opcode >> 3) & 0x6;
	uint32_t tmp = loc;

	if(reg == 0x6)
		tmp += sp;
	else
		tmp += (registers[reg] << 8) + registers[reg+1];

	registers[L] = tmp & 0xff;
	registers[H] = tmp >> 8;
	cy = (tmp > 0xffff);
} 

void si8080::rlc() {
	cy = registers[A] >> 7;
	registers[A] = (registers[A] << 1) + cy; 
}

void si8080::rrc() {
	cy = registers[A] & 0x1;
	registers[A] = (registers[A] >> 1) + (cy << 7);
}

void si8080::ral() {
	uint8_t c = cy;
	cy = registers[A] >> 7; 
	registers[A] = (registers[A] << 1) + c;
}

void si8080::rar() {
	uint8_t c = cy;
	cy = registers[A] & 1; 
	registers[A] = (registers[A] >> 1) + (c << 7);
}

void si8080::daa() {
    uint8_t lsb = registers[A] & 0xf;
    uint8_t msb = registers[A] >> 4;
	uint8_t total = 0;
	bool c = cy; //cy backup

    if (ac || lsb > 9) {
    	total += 0x6;
	}
    if (cy || msb > 9 || (msb >= 9 && lsb > 9)) {
    	total += 0x60;
	}

    registers[A] = setCond(registers[A] + total, registers[A], total);
	cy = (c) ? c : cy;
}

void si8080::shld() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	changeM(registers[L]);
	loc += 1;
	changeM(registers[H]);
}

void si8080::lhld() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	registers[L] = memory[loc];
	registers[H] = memory[loc+1];
}

void si8080::sta() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	changeM(registers[A]);
}

void si8080::lda() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	registers[A] = memory[loc];
}

void si8080::cma() {
	registers[A] = ~registers[A];
}

void si8080::stc() {
	cy = 1;
}

void si8080::cmc() {
	cy = !cy;
}

void si8080::mvi() {
	uint8_t reg = (opcode >> 3) & 0x7;
	if(reg == 0x6)
		changeM(memory[pc+1]);
	else 
		registers[reg] = memory[pc+1];
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

	if((opcode & 0xc0) == 0xc0)
		data = memory[pc+1];
	else if((opcode & 0x7) == 0x6)
		data = memory[loc];

	switch((opcode >> 3) & 0x7) {
		case 0x7:	setCond(registers[A] + (uint8_t)(~data) + 1, registers[A], (uint8_t)(~data)); cy = !cy; break;
		case 0x0:	registers[A] = setCond(registers[A] + data, registers[A], data); break;
		case 0x1: 	registers[A] = setCond(registers[A] + data + cy, registers[A], data); break;
		case 0x2:	registers[A] = setCond(registers[A] + (uint8_t)(~data) + 1, registers[A], (uint8_t)(~data)); cy = !cy; break;
		case 0x3:	registers[A] = setCond(registers[A] + (uint8_t)(~data) + !cy, registers[A], (uint8_t)(~data)); cy = !cy; break;
		case 0x4:{  uint8_t tmp = setCond(registers[A] & data, 0, 0);
				  	ac = ((registers[A] | data) & 0x8) != 0;
				  	cy = 0;
					registers[A] = tmp;
		} break;
		case 0x5:	registers[A] = setCond(registers[A] ^ data, 0, 0);
				  	ac = cy = 0;
		break;	
		case 0x6: 	registers[A] = setCond(registers[A] | data, 0, 0);
				  	ac = cy = 0;
		break; 
	}
}

void si8080::pop() { //11 rp(2) 0001
	uint8_t reg = (opcode >> 3) & 0x6;

	if(reg == 0x6) {
		uint8_t flags = memory[sp];
		s  = ((flags & 0x80) != 0);
		z  = ((flags & 0x40) != 0);
		ac = ((flags & 0x10) != 0);
		p  = ((flags &  0x4) != 0);
		cy = ((flags &  0x1) != 0);
		registers[A] = memory[sp+1];
	} else {
		registers[reg+1] = memory[sp];
		registers[reg] = memory[sp+1];
	}

	sp += 2;
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

void si8080::ret() {
	pc = (memory[sp+1] << 8) + memory[sp];
	sp += 2;
	cycles += 6;
}

void si8080::jmp() {
	pc = (memory[pc+2] << 8) + memory[pc+1];
}

void si8080::call() {
	memory[sp-2] = (pc + 3) & 0xff;
	memory[sp-1] = (pc + 3) >> 8;
	sp -= 2;
	pc = (memory[pc+2] << 8) + memory[pc+1];
	cycles += 6;
}

void si8080::retC() { //11 cond(3) 000
	if(checkCond())
		ret();
	else
		pc += 1;
}

void si8080::jmpC() { //11 cond(3) 010
	if(checkCond()) 
		jmp();
	else
		pc += 3;
}

void si8080::callC() { //11 cond(3) 100
	if(checkCond())
		call();
	else
		pc += 3;
}

void si8080::rst() { //11 exp(3) 111
	call();
	pc = (opcode & 0x38);

	hltB = false;
	interruptB = false;
}

void si8080::out() {
	loc = memory[pc+1];
	portOut[loc-2] = registers[A]; //2->0
}

void si8080::in() {
	loc = memory[pc+1];
}

void si8080::xthl() {
	uint16_t tmp = (memory[sp] << 8) + memory[sp+1];

	//replace 2 bytes on stack with H+L
	loc = sp;
	changeM(registers[L]);
	loc++;
	changeM(registers[H]);

	//replace H+L with 2 bytes from stack
	registers[L] = tmp >> 8;
	registers[H] = tmp & 0xff;
}

void si8080::xchg() {
	uint16_t tmp = (registers[E] << 8) + registers[D];
	registers[E] = registers[L];
	registers[D] = registers[H];
	registers[L] = tmp >> 8;
	registers[H] = tmp & 0xff;
}

void si8080::pchl() {
	pc = loc;
}

void si8080::sphl() {
	sp = loc;
}

void si8080::di() {
	interruptB = false;
}

void si8080::ei() {
	interruptB = true;
}

void si8080::cpm() {
	switch(registers[C]) {
		case 0x2: printf("%c", registers[E]); break;
		case 0x9: 
			for(uint16_t location = (registers[D] << 8) + registers[E]; memory[location] != 0x24; location++)
				printf("%c", memory[location]);
			break;
	}
	ret();
}

void si8080::end() { //warm boot
	runB = false;
}

void si8080::changeM(uint8_t value) { //it was getting in the way tbh
	if(loc >= vramStart && loc < (vramStart + 0x1C00)) {
		int offset = (loc - vramStart) * 8; //send this and the value over and copy the rest of the code below
		int x = offset / 256;			 //and update the screen every 1/60th of a second (but don't send the
		int y = 255 - (offset % 256);	 //pixels at that framerate)
		
		for(int i = 0; i < 8; i++) {
			bool bit = (value & (1 << i)) != 0;
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

	return ((check == 0x0 && !z) || (check == 0x1 && z) || (check == 0x2 && !cy) || (check == 0x3 && cy) || (check == 0x4 && !p) || (check == 0x5 && p) || (check == 0x6 && !s) || (check == 0x7 && s));
}

uint8_t si8080::setCond(uint16_t ans, uint8_t old, uint8_t diff) {
	ac = ((old ^ diff ^ ans) & 0x10) == 0x10;
	cy = (ans > 0xff);
	s = (ans & 0x80) == 0x80;
	z = (ans & 0xff) == 0;
	p = !__builtin_parity(ans & 0xff);

	return (ans & 0xff);
}

void si8080::load(string filename) {
	FILE* rom = fopen(filename.c_str(), "rb");
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
				pc = (cpmB) ? 0x100 : 0;
				uint32_t tmp = vramStart = pc + romSize + 0x400;

				if(cpmB) {
					for(uint32_t i = 0; i < pc; i++)
						memory.push_back(0);

					memory[0x0] = 0x10; //warm boot
					memory[0x5] = 0x20; //cpm call
					memory[0x6] = tmp & 0xff; //ram end lsb
					memory[0x7] = tmp >> 8; //ram end msb
					
					cpmPrint = fopen((filename + ".log").c_str(), "w");
				}

				tmp += 0x1C00;
				for(uint32_t i = pc; i < tmp; i++)
					memory.push_back(((i >= (romSize+pc)) ? 0 : buffer[i-pc]));

				cycles = cycBefore = interruptB = hltB = 0;

				fclose(rom);
				free(buffer);
			}
		}
	}
	
	//do space invaders stuff
	pixels = new uint8_t[224 * 256 * 3]; //vram is 224*32 bytes, each bit is a pixel
	
	for(uint32_t i = 0; i < 172032; i++)
		pixels[i] = 0;

	for(int i = 0; i < 8; i++)
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

	if(debugB)
		log = fopen("log.txt", "w");

	runB = true;
}