#include "si8080.h"

#define A 0x07
#define H 0x04
#define L 0x05

string si8080::load(const char* filename) {		
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

					memory[0x00] = 0x08; //warm boot
					memory[0x05] = 0x10; //cpm call
					uint16_t tmp = pc + romSize + 0x400;
					memory[0x06] = tmp & 0xff;
					memory[0x07] = (tmp >> 8) & 0xff;
					//memory[0x06] = 0x18; //"contains the value of FBASE and can be used to determine the size of available memory, assuming that the CCP is being overlaid by a transient program"
				}
				else
					pc = 0x0;

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
				debug = true;

				fclose(rom);
				free(buffer);
			}
		}
	}

	pixels = new uint32_t[224 * 256]; //vram is 224*32 bytes, each bit is a pixel

	for(int x = 0; x < 224; x++) { 
		for(int y = 0; y < 256; y++) { //2400, 2401.. bottom left to upper left then next row
			if(y < 16) {
				if(x < 16) {
					pixels[((255 - y) * 224) + x] = 0xFFFFFFFF; //white 
				}
				else if(x < 118) {
					pixels[((255 - y) * 224) + x] = 0x00FF00FF; //green
				}
				else if(x < 224) {
					pixels[((255 - y) * 224) + x] = 0xFFFFFFFF; //white
				}
			}
			else if(y < 72) {
				pixels[((255 - y) * 224) + x] = 0x00FF00FF; //green
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

	return "Loaded: " + (string)filename + "\nSize: " + to_string(romSize) + "\n";
}

void si8080::emulateCycle(bool external) {		
	if(!external)
		opcode = memory[pc];

	cycBefore = cycles;
	
	loc = ((uint16_t) registers[H] << 8) + registers[L];

	if(debug)
		cout << "PC: " << hex << setw(4) << +pc;
	
	switch((opcode >> 6) & 0x03) { //xx000000
		case 0x00: {
			switch(opcode & 0x07) { //00000xxx
				case 0x00: {
					switch((opcode >> 3) & 0x03) {
						case 0x00: break;
						case 0x01: break;
						case 0x02: cpm(registers[0x01]); break;
					}
				} break;
				case 0x01: {
					switch((opcode >> 3) & 0x01) { //00rpx001
						case 0x00: lxi(); break;
						case 0x01: dad(); break;
					}
				} break;
				case 0x02: {
					switch((opcode >> 3) & 0x07) { //00xrx010
						case 0x00: case 0x02: stax(); break;
						case 0x01: case 0x03: ldax(); break;
						case 0x04: //SHLD adr
							loc = (memory[pc+2] << 8) + memory[pc+1];
							changeM(registers[L]);
							loc += 1;
							changeM(registers[H]);

							pc += 2;
							cycles += 12;
							break;
						case 0x05: //LHLD adr
							loc = (memory[pc+2] << 8) + memory[pc+1];
							registers[L] = memory[loc];
							registers[H] = memory[loc+1];

							pc += 2;
							cycles += 12;
							break;
						case 0x06: //STA adr
							loc = ((memory[pc+2] << 8) + memory[pc+1]);
							changeM(registers[A]);

							pc += 2;
							cycles += 9;
							break;
						case 0x07: //LDA adr
							registers[A] = memory[(memory[pc+2] << 8) + memory[pc+1]];
							
							pc += 2;
							cycles += 9;
							break;
					}
				} break;
				case 0x03: {
					switch((opcode >> 3) & 0x01) { //00rpx011
						case 0x00: inx(); break;
						case 0x01: dcx(); break;
					}
				} break;
				case 0x04: inr(); break;
				case 0x05: dcr(); break;
				case 0x06: mvi(); break;
				case 0x07: {
					switch((opcode >> 3) & 0x07) { //00xxx111
						case 0x00: //RLC
							if((registers[A] & 0x80) == 0x80) {
								cy = 1;
								registers[A] = (registers[A] << 1) + 0x1; 
							}
							else {
								cy = 0;
								registers[A] = (registers[A] << 1);
							}
							break;
						case 0x01: //RRC
							if((registers[A] & 0x1) == 0x1) {
								cy = 1;
								registers[A] = (registers[A] >> 1) + 0x80; 
							}
							else {
								cy = 0;
								registers[A] = (registers[A] >> 1);
							}
							break;
						case 0x02: //RAL
							break;
						case 0x03: //RAR
							break;
						case 0x04: //DAA
							break;
						case 0x05: //CMA
							break;
						case 0x06: //STC
							break;
						case 0x07: //CMC
							break;
					}
				} break;
			}
		} break;

		case 0x01: {
			if(opcode == 0x76)
				hlt = true;
			else
				mov();
		} break;

		case 0x02: {
			math();
		} break;

		case 0x03: {
			switch(opcode & 0xf) {
				case 0x00: case 0x08: retC(); break;
				case 0x01: pop(); break;
				case 0x02: case 0x0a: jmpC(); break;
				case 0x03: {
					switch((opcode >> 4) & 0x03) {
						case 0x00: jmp(); break;
						case 0x01: //OUT D8
							port[memory[pc+1]+2] = registers[A];
							pc += 1;
							cycles += 6;
							break;
						case 0x02: //XTHL
						{
							uint16_t tmp3 = loc;
							uint8_t tmp2 = memory[sp];
							uint8_t tmp1 = memory[sp+1];
							loc = sp;
							changeM((tmp3 >> 4) & 0x0f);
							loc++;
							changeM(tmp3 & 0x0f);
							registers[L] = tmp2;
							registers[H] = tmp1;
						}
							break;
						case 0x03: //DI
							interrupt = false;
							break;
					}	
				} break;
				case 0x04: case 0x0c: callC(); break;
				case 0x05: push(); break;
				case 0x06: case 0x0e: math(); break;
				case 0x07: case 0x0f: rst(); break;
				case 0x09: {
					switch((opcode >> 4) & 0x03) {
						case 0x00: ret(); break;
						case 0x02: //PCHL
							pc = loc - 1;
							break;
						case 0x03: //SPHL
							sp = loc;
							break;
					}	
				} break;
				case 0x0b: {
					switch((opcode >> 4) & 0x03) {
						case 0x01: //IN D8
							registers[A] = port[memory[pc+1]];
							pc += 1;
							cycles += 6;
							break;
						case 0x02: //XCHG 
						{
							uint8_t tmp2 = registers[0x03];
							uint8_t tmp1 = registers[0x02];
							registers[0x03] = registers[L];
							registers[0x02] = registers[H];
							registers[L] = tmp2;
							registers[H] = tmp1;
						}
							break;
						case 0x03: //EI
							interrupt = true;
							break;
					}	
				} break;
				case 0x0d: call(); break;
			}
		} break;
	}

	if(debug) {
		cout << "\tSP: " << setw(4) << +sp << "\tA: " << +registers[A] << "\tB: " << +registers[0x00] << "\tC: " << +registers[0x01] << "\tD: " << +registers[0x02] << "\tE: " << +registers[0x03] << "\tH: " << +registers[H] << "\tL: " << +registers[L] << "\n";
		cout << "OP: " << hex << setw(4) << +opcode << "\tM: " << setw(4) << +loc << "\t\tCy: " << dec << +cy << "\tAC: " << +ac << "\tS: " << +s << "\tZ: " << +z << "\tP: " << +p << "\t20c0: " << +memory[0x20c0] << "\n\n";
	}

	pc++;
	cycles += 4;
}

bool si8080::checkCond() {
	uint8_t check = (opcode >> 3) & 0x07;

	if((check == 0x00 && z == 0) || (check == 0x01 && z) || (check == 0x02 && cy == 0) || (check == 0x03 && cy) || (check == 0x04 && p == 0) || (check == 0x05 && p) || (check == 0x06 && s == 0) || (check == 0x07 && s))
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
					pixels[(y * 224) - (i * 224) + x] = (bit) ? 0x00FF00FF : 0x0; //green
				else if(x < 224) 
					pixels[(y * 224) - (i * 224) + x] = (bit) ? 0xFFFFFFFF : 0x0; //white
			}
			else if(y > 184)
				pixels[(y * 224) - (i * 224) + x] = (bit) ? 0x00FF00FF : 0x0; //green
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
	uint8_t reg = (opcode >> 3) & 0x06;
	loc = ((uint16_t) memory[pc+2] << 8) + memory[pc+1];
	if(reg == 0x06) {
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
	uint8_t reg = (opcode >> 3) & 0x06;
	uint32_t tmp = loc;

	if(reg == 0x06) {
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
	uint8_t reg = (opcode >> 3) & 0x02;
	loc = (registers[reg] << 8) + registers[reg+1];
	changeM(registers[A]);
	
	cycles += 3;
}

void si8080::ldax() {
	uint8_t reg = (opcode >> 3) & 0x02;

	loc = (registers[reg] << 8) + registers[reg+1];
	registers[A] = memory[loc];
	
	cycles += 3;
}

void si8080::inx() {
	uint8_t reg = (opcode >> 3) & 0x06;
 	if(reg == 0x06) {
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
	uint8_t reg = (opcode >> 3) & 0x06;
	if(reg == 0x06) {
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
	uint8_t reg = (opcode >> 3) & 0x07;
	if(reg == 0x06) {
		changeM(setCond(memory[loc] + 1, memory[loc], 1, 0x4));
		cycles += 5;
	} 
	else
		registers[reg] = setCond(registers[reg] + 1, registers[reg], 1, 0x4);

	cycles += 1;
} 

void si8080::dcr() {
	uint8_t reg = (opcode >> 3) & 0x07;
	if(reg == 0x06) {
		changeM(setCond(memory[loc] - 1, memory[loc], 1, 0x4));
		cycles += 5;
	} 
	else
		registers[reg] = setCond(registers[reg] - 1, registers[reg], 1, 0x4);

	cycles += 1;
} 

void si8080::mvi() {
	uint8_t reg = (opcode >> 3) & 0x07;
	if(reg == 0x06) {
		changeM(memory[pc+1]);
		cycles += 3;
	} 
	else 
		registers[reg] = memory[pc+1];

	pc += 1;
	cycles += 3;
} 

void si8080::mov() {
	uint8_t dst = (opcode >> 3) & 0x07;
	uint8_t src = opcode & 0x07;

	if(dst == 0x06) {
		changeM(registers[src]);
		cycles += 2;
	}
	else if(src == 0x06) {
		registers[dst] = memory[loc];
		cycles += 2;
	}
	else {
		registers[dst] = registers[src];
	}

	cycles += 1;
}

void si8080::math() {
	uint8_t data = registers[opcode & 0x07];

	if(opcode > 0xc0) {
		data = memory[pc+1];
		pc += 1;
		cycles += 3;
	}
	else if((opcode & 0x07) == 0x06) {
		data = memory[loc];
		cycles += 3;
	}

	switch((opcode >> 3) & 0x07) {
		case 0x00: registers[A] = setCond(registers[A] + data, registers[A], data, 0x4); break;
		case 0x01: registers[A] = setCond(registers[A] + (data + cy), registers[A], (data + cy), 0x4); break;
		case 0x02: registers[A] = setCond(registers[A] - data, registers[A], data, 0x4); break;
		case 0x03: registers[A] = setCond(registers[A] - (data - cy), registers[A], (data + cy), 0x4); break;
		case 0x04: registers[A] = setCond(registers[A] & data, 0, 0, 0x2); break;
		case 0x05: registers[A] = setCond(registers[A] ^ data, 0, 0, 0x2); break;
		case 0x06: registers[A] = setCond(registers[A] | data, 0, 0, 0x2); break;
		case 0x07: setCond(registers[A] - data, registers[A], data, 0x4); break;
	}
}

void si8080::retC() { //11 cond(3) 000
	if(checkCond()) {
		ret();
	}

	cycles += 1;
}

void si8080::pop() { //11 rp(2) 0001
	uint8_t reg = (opcode >> 3) & 0x06;

	if(reg == 0x06) {
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
	uint8_t reg = (opcode >> 3) & 0x06;

	if(reg == 0x06) {
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
	cout << hex << +memory[sp+1] << +memory[sp] << endl;
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
	if(c == 9) {
		for(uint16_t location = (registers[0x02] << 8) + registers[0x03]; memory[location] != 0x24; location++) {
			cout << memory[location];
		}

		cout << "$" << endl;
	}

	ret();
}