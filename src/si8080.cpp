#include "si8080.h"

#define A 0x7
#define B 0x0
#define C 0x1
#define D 0x2
#define E 0x3
#define H 0x4
#define L 0x5

string si8080::load(const char* filename) {
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
				cycCount = 0;
				cycBefore = 0;

				interrupt = false;
				hlt = false;
				drawFlag = true;
				debug = false;

				fclose(rom);
				free(buffer);
			}
		}
	}

	return "Loaded: " + (string)filename + "\nSize: " + to_string(romSize) + "\n";
}

void si8080::emulateCycle(bool external) {
	if(!external)
		opcode = memory[pc];

	cycBefore = cycles;
	
	loc = ((uint16_t) registers[H] << 8) + registers[L];

	if(debug)
		cout << "PC: " << hex << setw(4) << +pc;
	
	switch((opcode >> 6) & 0x3) { //xx000000
		case 0x0: {
			switch(opcode & 0x7) { //00000xxx
				case 0x0: {
					switch((opcode >> 3) & 0x3) {
						case 0x0: break; //nop
						case 0x1: break; //warm boot
						case 0x2: cpm(registers[C]); break; //cpm 5
					}
				} break;
				case 0x1: {
					switch((opcode >> 3) & 0x1) { //00rpx001
						case 0x0: lxi(); break;
						case 0x1: dad(); break;
					}
				} break;
				case 0x2: {
					switch((opcode >> 3) & 0x7) { //00xrx010
						case 0x0: case 0x2: stax(); break;
						case 0x1: case 0x3: ldax(); break;
						case 0x4: //SHLD adr
							loc = (memory[pc+2] << 8) + memory[pc+1];
							changeM(registers[L]);
							loc += 1;
							changeM(registers[H]);

							cout << loc << endl;
							cout << hex << +memory[loc] << " " << +memory[loc-1] << endl;

							pc += 2;
							cycles += 12;
							break;
						case 0x5: //LHLD adr
							loc = (memory[pc+2] << 8) + memory[pc+1];
							registers[L] = memory[loc];
							registers[H] = memory[loc+1];

							pc += 2;
							cycles += 12;
							break;
						case 0x6: //STA adr
							loc = ((memory[pc+2] << 8) + memory[pc+1]);
							changeM(registers[A]);

							pc += 2;
							cycles += 9;
							break;
						case 0x7: //LDA adr
							registers[A] = memory[(memory[pc+2] << 8) + memory[pc+1]];
							
							pc += 2;
							cycles += 9;
							break;
					}
				} break;
				case 0x3: {
					switch((opcode >> 3) & 0x1) { //00rpx011
						case 0x0: inx(); break;
						case 0x1: dcx(); break;
					}
				} break;
				case 0x4: inr(); break;
				case 0x5: dcr(); break;
				case 0x6: mvi(); break;
				case 0x7: {
					switch((opcode >> 3) & 0x7) { //00xxx111
						case 0x0: //RLC
							if((registers[A] & 0x80) == 0x80) {
								cy = 1;
								registers[A] = (registers[A] << 1) + 0x1; 
							}
							else {
								cy = 0;
								registers[A] = (registers[A] << 1);
							}
							break;
						case 0x1: //RRC
							if((registers[A] & 0x1) == 0x1) {
								cy = 1;
								registers[A] = (registers[A] >> 1) + 0x80; 
							}
							else {
								cy = 0;
								registers[A] = (registers[A] >> 1);
							}
							break;
						case 0x2: //RAL
							break;
						case 0x3: //RAR
							break;
						case 0x4: //DAA
							break;
						case 0x5: //CMA
							break;
						case 0x6: //STC
							break;
						case 0x7: //CMC
							break;
					}
				} break;
			}
		} break;

		case 0x1: {
			if(opcode == 0x76)
				hlt = true;
			else
				mov();
		} break;

		case 0x2: {
			math();
		} break;

		case 0x3: {
			switch(opcode & 0xf) {
				case 0x0: case 0x8: retC(); break;
				case 0x1: pop(); break;
				case 0x2: case 0xa: jmpC(); break;
				case 0x3: {
					switch((opcode >> 4) & 0x3) {
						case 0x0: jmp(); break;
						case 0x1: //OUT D8
							port[memory[pc+1]+2] = registers[A];
							pc += 1;
							cycles += 6;
							break;
						case 0x2: //XTHL
						{
							uint16_t tmp3 = loc;
							uint8_t tmp2 = memory[sp];
							uint8_t tmp1 = memory[sp+1];
							loc = sp;
							changeM((tmp3 >> 8) & 0xf);
							loc++;
							changeM(tmp3 & 0xf);
							registers[L] = tmp2;
							registers[H] = tmp1;
						}
							break;
						case 0x3: //DI
							interrupt = false;
							break;
					}	
				} break;
				case 0x4: case 0xc: callC(); break;
				case 0x5: push(); break;
				case 0x6: case 0xe: math(); break;
				case 0x7: case 0xf: rst(); break;
				case 0x9: {
					switch((opcode >> 4) & 0x3) {
						case 0x0: ret(); break;
						case 0x2: //PCHL
							pc = loc - 1;
							break;
						case 0x3: //SPHL
							sp = loc;
							break;
					}	
				} break;
				case 0xb: {
					switch((opcode >> 4) & 0x3) {
						case 0x1: //IN D8
							registers[A] = port[memory[pc+1]];
							pc += 1;
							cycles += 6;
							break;
						case 0x2: //XCHG 
						{
							uint8_t tmp2 = registers[E];
							uint8_t tmp1 = registers[D];
							registers[E] = registers[L];
							registers[D] = registers[H];
							registers[L] = tmp2;
							registers[H] = tmp1;
						}
							break;
						case 0x3: //EI
							interrupt = true;
							break;
					}	
				} break;
				case 0xd: call(); break;
			}
		} break;
	}

	if(debug) {
		cout << "\tSP: " << setw(4) << +sp << "\tA: " << +registers[A] << "\tB: " << +registers[B] << "\tC: " << +registers[C] << "\tD: " << +registers[D] << "\tE: " << +registers[E] << "\tH: " << +registers[H] << "\tL: " << +registers[L] << "\n";
		cout << "OP: " << hex << setw(4) << +opcode << "\tM: " << setw(4) << +loc << "\t\tCy: " << dec << +cy << "\tAC: " << +ac << "\tS: " << +s << "\tZ: " << +z << "\tP: " << +p << "\n\n";
	}

	pc++;
	cycles += 4;
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
	return (~((old ^ diff) & 0x10) == (ans & 0x10));
}
else {
	return ((old & 0x8) == 0 && (diff & 0x8) == 1);
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

void si8080::stax() {
	uint8_t reg = (opcode >> 3) & 0x2;
	loc = (registers[reg] << 8) + registers[reg+1];
	changeM(registers[A]);
	
	cycles += 3;
}

void si8080::ldax() {
	uint8_t reg = (opcode >> 3) & 0x2;

	loc = (registers[reg] << 8) + registers[reg+1];
	registers[A] = memory[loc];
	
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

	// cout << "POP from " << hex << +sp << ": ";
	// for(int i = sp; i < vramStart; i++) {
	// 	cout << "|" << hex << +memory[i];
	// }

	// cout << endl;
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

	// cout << "PUSH: ";
	// for(int i = sp; i < vramStart; i++) {
	// 	cout << "|" << hex << +memory[i];
	// }

	// cout << endl;
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

void si8080::cpm(uint8_t c) {
	if(c == 2) {
		printf("%c", memory[registers[E]]);
	}

	if(c == 9) {
		for(uint16_t location = (registers[D] << 8) + registers[E]; memory[location] != 0x24; location++) {
			printf("%c", memory[location]);
		}

		cout << "$";
	}

	ret();
}