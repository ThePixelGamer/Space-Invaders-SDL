#include "si8080.h"

#define A 0x7
#define B 0x0
#define C 0x1
#define D 0x2
#define E 0x3
#define H 0x4
#define L 0x5


void si8080::load(const char* filename) {
	pixels = new uint32_t[224 * 256]; //vram is 224*32 bytes, each bit is a pixel

	for(int x = 0; x < 224; x++) { 
		for(int y = 0; y < 256; y++) { //2400, 2401.. bottom left to upper left then next row
			if(y < 16) {
				if(x < 16) {
					pixels[((255 - y) * 224) + x] = 0xFFFFFFFF; //white 
				}
				else if(x < 118) {
					pixels[((255 - y) * 224) + x] = 0x0FF00FF; //green
				}
				else if(x < 224) {
					pixels[((255 - y) * 224) + x] = 0xFFFFFFFF; //white
				}
			}
			else if(y < 72) {
				pixels[((255 - y) * 224) + x] = 0x0FF00FF; //green
			}
			else if(y < 192) {
				pixels[((255 - y) * 224) + x] = 0xFFFFFFFF; //white
			}
			else if(y < 224) {
				pixels[((255 - y) * 224) + x] = 0xFF0000FF; //red
			}
			else if(y < 256) {
				pixels[((255 - y) * 224) + x] = 0xFFFFFFFF; //white
			}
		}
	}

	for(int i = 0; i < 8; i++)
		registers[i] = 0;

	FILE* rom = fopen(filename, "rb");
	if (rom == NULL) {
		fputs("File error", stderr); 
	}
	else {
		fseek(rom, 0, SEEK_END);
		romSize = ftell(rom);
		rewind(rom);
		
		char* buffer = (char*)malloc(sizeof(char) * romSize);
		if (buffer == NULL) {
			fputs("Memory error", stderr);
		}
		else {
			uint16_t result = fread(buffer, 1, romSize, rom);
			if (result != romSize) {
				fputs("Reading error", stderr); 
			}
			else {
				if(cmp) {
					pc = 0x100;
					for(int i = 0; i < pc; i++)
						memory.push_back(0);

					memory[0x0] = 0x8; //warm boot
					memory[0x5] = 0x10; //cpm call
					uint16_t tmp = pc + romSize + 0x2000;
					memory[0x6] = tmp & 0xff;
					memory[0x7] = (tmp >> 8) & 0xff;
				}
				else {
					pc = 0x0;
				}

				for(int i = pc; i < (pc + romSize); i++)
					memory.push_back(buffer[i-pc]);

				for(int i = (pc + romSize); i < (pc + romSize + 0x2000); i++)
					memory.push_back(0);	

				vramStart = pc + romSize + 0x400;
				sp = vramStart;
				
				cycles = 0;
				cycBefore = 0;

				interrupt = false;
				hlt = false;
				drawFlag = true;
				debug = true;

				fclose(rom);
				free(buffer);
			}
		}
	}

	log = fopen("log.txt", "w");
	fprintf(log, "Loaded: %s\nSize: %li\n", filename, romSize);
}

void si8080::emulateCycle() {
	opcode = memory[pc];
	loc = ((uint16_t) registers[H] << 8) + registers[L];

	cycles += 4;

	if(debug)
		fprintf(log, "PC: %4X", pc);
	
	switch(opcode) {
	//0x00-0x3f
		case 0x0:															break; 
					case 0x10:							cpm();				break; //cpm 5
		case 0x1:	case 0x11:	case 0x21:	case 0x31:	lxi();				break;
		case 0x2:	case 0x12:							stax(); 			break; 
								case 0x22:				shld();				break;
											case 0x32:	sta();				break;
		case 0x3:	case 0x13:	case 0x23:	case 0x33:	inx();				break;
		case 0x4:	case 0x14:	case 0x24:	case 0x34:	inr();				break; //0xc same thing
		case 0x5:	case 0x15:	case 0x25:	case 0x35:	dcr();				break;
		case 0x6:	case 0x16:	case 0x26:	case 0x36:	mvi();				break;
		case 0x7:										rlc();				break;
					case 0x17:							ral();				break;
								case 0x27:				daa();				break;
											case 0x37:	stc();				break;
		case 0x8: 															break; //warm boot
		case 0x9:	case 0x19:	case 0x29:	case 0x39: 	dad();				break;
		case 0xa:	case 0x1a:							ldax(); 			break;
								case 0x2a:				lhld();				break;
											case 0x3a:	lda();				break;
		case 0xb:	case 0x1b:	case 0x2b:	case 0x3b:	dcx();				break;
		case 0xc:	case 0x1c:	case 0x2c:	case 0x3c:	inr();				break;
		case 0xd:	case 0x1d:	case 0x2d:	case 0x3d:	dcr();				break;
		case 0xe:	case 0x1e:	case 0x2e:	case 0x3e:	mvi();				break;
		case 0xf:										rrc();				break;
					case 0x1f:							rar();				break;
								case 0x2f:				cma();				break;
											case 0x3f:	cmc();				break;
		
	//0x40-0x7f
		case 0x40:	case 0x50:	case 0x60:	case 0x70:
		case 0x41:	case 0x51:	case 0x61:	case 0x71:
		case 0x42:	case 0x52:	case 0x62:	case 0x72:
		case 0x43:	case 0x53:	case 0x63:	case 0x73:
		case 0x44:	case 0x54:	case 0x64:	case 0x74:
		case 0x45:	case 0x55:	case 0x65:	case 0x75:
		case 0x46:	case 0x56:	case 0x66:
		case 0x47:	case 0x57:	case 0x67:	case 0x77:
		case 0x48:	case 0x58:	case 0x68:	case 0x78:
		case 0x49:	case 0x59:	case 0x69:	case 0x79:
		case 0x4a:	case 0x5a:	case 0x6a:	case 0x7a:
		case 0x4b:	case 0x5b:	case 0x6b:	case 0x7b:
		case 0x4c:	case 0x5c:	case 0x6c:	case 0x7c:
		case 0x4d:	case 0x5d:	case 0x6d:	case 0x7d:
		case 0x4e:	case 0x5e:	case 0x6e:	case 0x7e:
		case 0x4f:	case 0x5f:	case 0x6f:	case 0x7f:	mov();				break;
											case 0x76: 	hlt = true;			break;

	//0x80-0xbf
		case 0x80:	case 0x90:	case 0xa0:	case 0xb0:
		case 0x81:	case 0x91:	case 0xa1:	case 0xb1:
		case 0x82:	case 0x92:	case 0xa2:	case 0xb2:
		case 0x83:	case 0x93:	case 0xa3:	case 0xb3:
		case 0x84:	case 0x94:	case 0xa4:	case 0xb4:
		case 0x85:	case 0x95:	case 0xa5:	case 0xb5:
		case 0x86:	case 0x96:	case 0xa6:	case 0xb6:
		case 0x87:	case 0x97:	case 0xa7:	case 0xb7:
		case 0x88:	case 0x98:	case 0xa8:	case 0xb8:
		case 0x89:	case 0x99:	case 0xa9:	case 0xb9:
		case 0x8a:	case 0x9a:	case 0xaa:	case 0xba:
		case 0x8b:	case 0x9b:	case 0xab:	case 0xbb:
		case 0x8c:	case 0x9c:	case 0xac:	case 0xbc:
		case 0x8d:	case 0x9d:	case 0xad:	case 0xbd:
		case 0x8e:	case 0x9e:	case 0xae:	case 0xbe:
		case 0x8f:	case 0x9f:	case 0xaf:	case 0xbf:	math();				break;

	//0xc0-0xff
		case 0xc0:	case 0xd0:	case 0xe0:	case 0xf0:	retC();				break;
		case 0xc1:	case 0xd1:	case 0xe1:	case 0xf1:	pop();				break;
		case 0xc2:	case 0xd2:	case 0xe2:	case 0xf2:	jmpC();				break;
		case 0xc3:										jmp();				break;
					case 0xd3:							out();				break;
								case 0xe3:				xthl();				break;
											case 0xf3:	interrupt = false;	break;
		case 0xc4:	case 0xd4:	case 0xe4:	case 0xf4:	callC();			break;
		case 0xc5:	case 0xd5:	case 0xe5:	case 0xf5:	push();				break;
		case 0xc6:	case 0xd6:	case 0xe6:	case 0xf6:	math();				break;
		case 0xc7:	case 0xd7:	case 0xe7:	case 0xf7:	rst();				break;
		case 0xc8:	case 0xd8:	case 0xe8:	case 0xf8:	retC();				break;
		case 0xc9:										ret();				break;
								case 0xe9:				pc = loc - 1;		break;
											case 0xf9:	sp = loc;			break;
		case 0xca:	case 0xda:	case 0xea:	case 0xfa:	jmpC();				break;
					case 0xdb:							in();				break;
								case 0xeb:				xchg();				break;
											case 0xfb:	interrupt = true;	break;
		case 0xcc:	case 0xdc:	case 0xec:	case 0xfc:	callC();			break;
		case 0xcd:										call();				break;
		case 0xce:	case 0xde:	case 0xee:	case 0xfe:	math();				break;
		case 0xcf:	case 0xdf:	case 0xef:	case 0xff:	rst();				break;

		default:
			cout << hex << +opcode << endl;
	}

	if(debug) {
		fprintf(log, "\tSP: %4X\tA: %2X\tB: %2X\tC: %2X\tD: %2X\tE: %2X\tH: %2X\tL: %2X\n", sp, registers[A], registers[B], registers[C], registers[D], registers[E], registers[H], registers[L]);
		fprintf(log, "OP: %4X\tM: %4X\t\tCy: %i\tAC: %i\tS: %i\tZ: %i\tP: %i\tCycles: %i\n\n", opcode, loc, cy, ac, s, z, p, cycles);
	} 

	pc++;
}

bool si8080::checkCond() {
	uint8_t check = (opcode >> 3) & 0x7;

	if((check == 0x0 && z == 0) || (check == 0x1 && z) || (check == 0x2 && cy == 0) || (check == 0x3 && cy) || (check == 0x4 && p == 0) || (check == 0x5 && p) || (check == 0x6 && s == 0) || (check == 0x7 && s))
		return true;

	return false;
}

//flags - 0x2: logic 0x4: math
uint8_t si8080::setCond(uint16_t ans, uint8_t old, uint8_t diff, uint8_t flags) {
	if((flags & 0x2) == 0x2) {
		cy = 0;
		ac = 0;
		s = ((registers[A] & 0x80) == 0x80);
		z = (registers[A] == 0);
		p = checkParity(registers[A] & 0xff);
	}
	if((flags & 0x4) == 0x4) {
		cy = (ans > 0xff);
		ac = (((old ^ diff ^ ans) & 0x10) == 0x10);
		s = ((ans & 0x80) == 0x80);
		z = ((ans & 0xff) == 0);
		p = checkParity(ans & 0xff);
	}

	return ans & 0xff;
}

uint8_t si8080::checkParity(uint8_t ans) {
	bitset<8> bs(ans);
	int count = 0;
	for(unsigned int i = 0; i < bs.to_string().size(); i++) {
		if(bs.to_string().substr(i, 1).compare("1") == 0) 
			count++;
	}

	return (count % 2 == 1);
}

/*if(ans >= old) {
	return ((old ^ diff ^ ans) & 0x10) == 0x10;
}
else {
	return (~(old ^ diff ^ ans) & 0x10) == 0x10;
} my implementation of the ac bit*/

void si8080::changeM(uint8_t value) { //it's right now :D
	if(loc >= vramStart && loc < vramStart + 0x1C00) {
		int offset = (loc - vramStart) * 8; 
		int x = offset / 256;
		int y = 255 - (offset % 256);
		
		for(int i = 0; i < 8; i++) {
			bool bit = (((value >> i) & 0x1) == 0x1);
			
			if(y > 240) {
				if(x < 16) 
					pixels[(y * 224) - (i * 224) + x] = (bit) ? 0xFFFFFFFF : 0x0; //white 
				else if(x < 118) 
					pixels[(y * 224) - (i * 224) + x] = (bit) ? 0x0FF00FF : 0x0; //green
				else if(x < 224) 
					pixels[(y * 224) - (i * 224) + x] = (bit) ? 0xFFFFFFFF : 0x0; //white
			}
			else if(y > 184)
				pixels[(y * 224) - (i * 224) + x] = (bit) ? 0x0FF00FF : 0x0; //green
			else if(y > 64)
				pixels[(y * 224) - (i * 224) + x] = (bit) ? 0xFFFFFFFF : 0x0;//white
			else if(y > 32)
				pixels[(y * 224) - (i * 224) + x] = (bit) ? 0xFF0000FF : 0x0; //red
			else if(y > 0)
				pixels[(y * 224) - (i * 224) + x] = (bit) ? 0xFFFFFFFF : 0x0; //white
		}
		
		drawFlag = true;
	}

	memory[loc] = value;
} 

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

	pc += 2;
	cycles += 6;
} 

void si8080::stax() {
	uint8_t reg = (opcode >> 3) & 0x2;
	loc = (registers[reg] << 8) + registers[reg+1];
	changeM(registers[A]);
	
	cycles += 3;
}

void si8080::inx() {
	uint8_t reg = (opcode >> 3) & 0x6;
 	if(reg == 0x6) {
		sp += 1;
	}
	else {
		loc = ((registers[reg] << 8) + registers[reg+1]) + 1;
		registers[reg+1] = loc & 0xff;
		registers[reg] = (loc >> 8) & 0xff;
	}
	
	cycles += 1;
}
				
void si8080::inr() {
	uint8_t reg = (opcode >> 3) & 0x7;
	if(reg == 0x6) {
		changeM(setCond(memory[loc] + 1, memory[loc], 1, 0x4));
		cycles += 5;
	} 
	else
		registers[reg] = setCond(registers[reg] + 1, registers[reg], 1, 0x4);

	cycles += 1;
} 

void si8080::dcr() {
	uint8_t reg = (opcode >> 3) & 0x7;
	if(reg == 0x6) {
		changeM(setCond(memory[loc] - 1, memory[loc], 1, 0x4));
		cycles += 5;
	} 
	else
		registers[reg] = setCond(registers[reg] - 1, registers[reg], 1, 0x4);

	cycles += 1;
} 

void si8080::mvi() {
	uint8_t reg = (opcode >> 3) & 0x7;
	if(reg == 0x6) {
		changeM(memory[pc+1]);
		cycles += 3;
	} 
	else 
		registers[reg] = memory[pc+1];

	pc += 1;
	cycles += 3;
} 

void si8080::rlc() {
	if((registers[A] & 0x80) == 0x80) {
		cy = 1;
		registers[A] = (registers[A] << 1) + 0x1; 
	}
	else {
		cy = 0;
		registers[A] = (registers[A] << 1);
	}
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
	cycles += 6;
} 

void si8080::ldax() {
	uint8_t reg = (opcode >> 3) & 0x2;

	loc = (registers[reg] << 8) + registers[reg+1];
	registers[A] = memory[loc];
	
	cycles += 3;
}

void si8080::dcx() {
	uint8_t reg = (opcode >> 3) & 0x6;
	if(reg == 0x6) {
		sp -= 1;
	}
	else {
		loc = ((registers[reg] << 8) + registers[reg+1]) - 1;
		registers[reg+1] = loc & 0xff;
		registers[reg] = (loc >> 8) & 0xff;
	}

	cycles += 1;
}

void si8080::rrc() {
	if((registers[A] & 0x1) == 0x1) {
		cy = 1;
		registers[A] = (registers[A] >> 1) + 0x80; 
	}
	else {
		cy = 0;
		registers[A] = (registers[A] >> 1);
	}
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
	cycles += 12;
}

void si8080::daa() {
    uint8_t lsb = registers[A] & 0xf;
    uint8_t msb = registers[A] >> 4;

    if (ac || lsb > 9) {
        setCond(registers[A] + 0x6, registers[A], 0x6, 0x4);
    }
    if (cy || msb > 9 || (msb >= 9 && lsb > 9)) {
        setCond(registers[A] + 0x60, registers[A], 0x60, 0x4);
        cy = 1;
    }
    p = checkParity(registers[A]);
}

void si8080::lhld() {
	loc = (memory[pc+2] << 8) + memory[pc+1];
	registers[L] = memory[loc];
	registers[H] = memory[loc+1];

	pc += 2;
	cycles += 12;
}

void si8080::cma() {
	registers[A] = !registers[A];
}

void si8080::sta() {
	loc = ((memory[pc+2] << 8) + memory[pc+1]);
	changeM(registers[A]);

	pc += 2;
	cycles += 9;
}

void si8080::stc() {
	cy = 1;
}

void si8080::lda() {
	registers[A] = memory[(memory[pc+2] << 8) + memory[pc+1]];
	
	pc += 2;
	cycles += 9;
}

void si8080::cmc() {
	cy = !cy;
}

void si8080::mov() {
	uint8_t dst = (opcode >> 3) & 0x7;
	uint8_t src = opcode & 0x7;

	if(dst == 0x6) {
		changeM(registers[src]);
		cycles += 2;
	}
	else if(src == 0x6) {
		registers[dst] = memory[loc];
		cycles += 2;
	}
	else {
		registers[dst] = registers[src];
	}

	cycles += 1;
}

void si8080::math() {
	uint8_t data = registers[opcode & 0x7];

	if(opcode > 0xc0) {
		data = memory[pc+1];
		pc += 1;
		cycles += 3;
	}
	else if((opcode & 0x7) == 0x6) {
		data = memory[loc];
		cycles += 3;
	}

	switch((opcode >> 3) & 0x7) {
		case 0x0: registers[A] = setCond(registers[A] + data, registers[A], data, 0x4); break;
		case 0x1: registers[A] = setCond(registers[A] + (data + cy), registers[A], (data + cy), 0x4); break;
		case 0x2: registers[A] = setCond(registers[A] - data, registers[A], data, 0x4); break;
		case 0x3: registers[A] = setCond(registers[A] - (data - cy), registers[A], (data + cy), 0x4); break;
		case 0x4: registers[A] = setCond(registers[A] & data, 0, 0, 0x2); break;
		case 0x5: registers[A] = setCond(registers[A] ^ data, 0, 0, 0x2); break;
		case 0x6: registers[A] = setCond(registers[A] | data, 0, 0, 0x2); break;
		case 0x7: setCond(registers[A] - data, registers[A], data, 0x4); break;
	}
}

void si8080::retC() { //11 cond(3) 000
	if(checkCond()) {
		ret();
	}

	cycles += 1;
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
	}
	else {
		registers[reg+1] = memory[sp];
		registers[reg] = memory[sp+1];
	}

	sp += 2;
	cycles += 6;
}

void si8080::jmpC() { //11 cond(3) 010
	if(checkCond()) {
		jmp();
	}
	else {
		pc += 2;
	}
}

void si8080::callC() { //11 cond(3) 100
	if(checkCond()) {
		call();
	}
	else {
		cycles += 7;
		pc += 2;
	}
}

void si8080::push() { //11 rp(2) 0101
	uint8_t reg = (opcode >> 3) & 0x6;

	if(reg == 0x6) { //PSW
		memory[sp-2] = (s << 7) + (z << 6) + (ac << 4) + (p << 2) + 0x1 + cy;
		memory[sp-1] = registers[A];
	}
	else {
		memory[sp-2] = registers[reg+1];
		memory[sp-1] = registers[reg];
	}

	sp -= 2;
	cycles += 7;
}

void si8080::rst() { //11 exp(3) 111
	uint8_t reg = opcode & 0x38;
	memory[sp-2] = pc & 0xff;
	memory[sp-1] = (pc & 0xff00) >> 8;
	sp -= 2;

	pc = reg - 1;
	cycles += 7;
	hlt = false;
	interrupt = false;
}

void si8080::jmp() {
	pc = (memory[pc+2] << 8) + memory[pc+1] - 1;
	cycles += 6;
}

void si8080::ret() {
	pc = (memory[sp+1] << 8) + memory[sp] + 2;
	sp += 2;
	cycles += 6;
}

void si8080::call() {
	memory[sp-2] = pc & 0xff;
	memory[sp-1] = (pc & 0xff00) >> 8;
	sp -= 2;

	pc = (memory[pc+2] << 8) + memory[pc+1] - 1;
	cycles += 13;
}

void si8080::out() {
	port[memory[pc+1]+2] = registers[A];
	pc += 1;
	cycles += 6;
}

void si8080::in() {
	registers[A] = port[memory[pc+1]];
	pc += 1;
	cycles += 6;
}

void si8080::xthl() {
	uint16_t tmp3 = loc;
	uint8_t tmp2 = memory[sp];
	uint8_t tmp1 = memory[sp+1];
	loc = sp;
	changeM((tmp3 >> 8) & 0xf);
	loc++;
	changeM(tmp3 & 0xf);
	registers[L] = tmp2;
	registers[H] = tmp1;
	cycles += 14;
}

void si8080::xchg() {
	uint8_t tmp2 = registers[E];
	uint8_t tmp1 = registers[D];
	registers[E] = registers[L];
	registers[D] = registers[H];
	registers[L] = tmp2;
	registers[H] = tmp1;
}

							

void si8080::cpm() {
	switch(registers[C]) {
		case 0x2: printf("%c", memory[registers[E]]); break;

		case 0x9: 
			for(uint16_t location = (registers[D] << 8) + registers[E]; memory[location] != 0x24; location++) {
				printf("%c", memory[location]);
			}

			cout << "$";
			break;
	}

	ret();
}