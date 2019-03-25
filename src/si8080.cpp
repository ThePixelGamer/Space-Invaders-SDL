#include "si8080.h"

si8080::si8080() {
	pc		= 0x200;	
	sp		= 0;		

	for(int i = 0; i < 2048; i++)
		pixels[i] = 0;

	for(int i = 0; i < 4096; i++)
		memory[i] = 0;	

	drawFlag = true;
}

void si8080::emulateCycle() {
	uint8_t *opcode = memory[pc];//f u
	
	switch(*opcode & 0xF0) {
		case 0x00: //NOP
			break;

		case 0x01: //LXI B,D16
			c = opcode[1];
			b = opcode[2];	
			pc += 2;	
			break;

		case 0x02: //STAX B
			memory[((uint16_t) b << 8) + c] = &a;
			break;
			
		case 0x03: //INX B
		{
			uint16_t tmp = ((uint16_t) b << 8) + (c + 1);
			c = (tmp & 0xff);
			b = (tmp >> 8);
		}
			break;
			
		case 0x04: //INR B
			b = (setCond((uint32_t) b + 1, 1, b, 0xf)) & 0xff;
			break;
			
		case 0x05: //DCR B
			b = (setCond((uint32_t) b - 1, -1, b, 0xf)) & 0xff;	
			break;

		case 0x06: //MVI B, D8
			b = opcode[1];
			pc += 1;	
			break;
			
		case 0x07: //RLC
			if(a & 0x80 == 0x80) {
				cy = 1;
				a = (a << 1) + 0x1; 
			}
			else {
				cy = 0;
				a = (a << 1);
			}
			break;
			
		case 0x09: //DAD B
			uint16_t tmp = setCond((((uint32_t) b << 8) + c) + (((uint32_t) h << 8) + l), ((uint16_t) b << 8) + c, ((uint16_t) h << 8) + l, 0x10);
			h = (tmp & 0xff00) >> 8;
			l = tmp & 0xff;
			break;
			
		case 0x0a: //LDAX B
			a = memory[(b << 8) + c];
			break;
			
		case 0x0b: //DCX B
			{
				uint16_t tmp = ((uint16_t) b << 8) + c - 1;
				c = (tmp & 0xff);
				b = (tmp >> 8);
			}
			break;
			
		case 0x0c: //INR C
			c = (setCond((uint32_t) c + 1, 1, c, 0xf)) & 0xff;	
			break;
			
		case 0x0d: //DCR C
			c = (setCond((uint32_t) c - 1, -1, c, 0xf)) & 0xff;	
			break;
			
		case 0x0e: //MVI C,D8
			c = opcode[1];
			pc += 1;	
			break;
			
		case 0x0f: //RRC
			if(a & 0x1 == 0x1) {
				cy = 1;
				a = (a >> 1) + 0x80; 
			}
			else {
				cy = 0;
				a = (a >> 1);
			}
			break;

		case 0x11: //LXI D,D16
			e = opcode[1];
			d = opcode[2];	
			pc += 2;
			break;

		case 0x12: //STAX D
			memory[(d << 8) + e] = &a;
			break;

		case 0x13: //INX D
			uint16_t tmp = ((uint16_t) d << 8) + (e + 1);
			e = (tmp & 0xff);
			d = (tmp >> 8);
			break;

		case 0x14: //not done
			break;

		case 0x15: //not done
			break;

		case 0x16: //not done
			break;

		case 0x17: //not done
			break;

		case 0x19: //not done
			break;

		case 0x1a: //not done
			break;

		case 0x1b: //not done
			break;

		case 0x1c: //not done
			break;

		case 0x1d: //not done
			break;

		case 0x1e: //not done
			break;

		case 0x1f: //not done
			break;

		case 0x21: //not done
			break;

		case 0x22: //not done
			break;

		case 0x23: //not done
			break;

		case 0x24: //not done
			break;

		case 0x25: //not done
			break;

		case 0x26: //not done
			break;

		case 0x27: //wolfy don't even dare to fucking attempt this one lmao (it's not needed and I'll do it later)
			break;

		case 0x29: //DAD H
			uint16_t tmp = setCond((((uint32_t) h << 8) + l) << 1, ((uint16_t) h << 8) + l, ((uint16_t) h << 8) + l, 0x10);
			h = (tmp & 0xff00) >> 8;
			l = tmp & 0xff;
			break;

		case 0x2a: //not done
			break;

		case 0x2b: //not done
			break;

		case 0x2c: //not done
			break;

		case 0x2d: //not done
			break;

		case 0x2e: //not done
			break;

		case 0x2f: //not done
			break;

		case 0x31: //not done
			break;

		case 0x32: //not done
			break;

		case 0x33: //not done
			break;

		case 0x34: //not done
			break;

		case 0x35: //not done
			break;

		case 0x36: //not done
			break;

		case 0x37: //not done
			break;

		case 0x39: //not done
			break;

		case 0x3a: //not done
			break;

		case 0x3b: //not done
			break;

		case 0x3c: //not done
			break;

		case 0x3d: //not done
			break;

		case 0x3e: //not done
			break;

		case 0x3f: //not done
			break;

		case 0x40: //not done
			break;

		case 0x41: //not done
			break;

		case 0x42: //not done
			break;

		case 0x43: //not done
			break;

		case 0x44: //not done
			break;

		case 0x45: //not done
			break;

		case 0x46: //not done
			break;

		case 0x47: //not done
			break;

		case 0x48: //not done
			break;

		case 0x49: //not done
			break;

		case 0x4a: //not done
			break;

		case 0x4b: //not done
			break;

		case 0x4c: //not done
			break;

		case 0x4d: //not done
			break;

		case 0x4e: //not done
			break;

		case 0x4f: //not done
			break;

		case 0x50: //not done
			break;

		case 0x51: //not done
			break;

		case 0x52: //not done
			break;

		case 0x53: //not done
			break;

		case 0x54: //not done
			break;

		case 0x55: //not done
			break;

		case 0x56: //not done
			break;

		case 0x57: //not done
			break;

		case 0x58: //not done
			break;

		case 0x59: //not done
			break;

		case 0x5a: //not done
			break;

		case 0x5b: //not done
			break;

		case 0x5c: //not done
			break;

		case 0x5d: //not done
			break;

		case 0x5e: //not done
			break;

		case 0x5f: //not done
			break;

		case 0x60: //not done
			break;

		case 0x61: //not done
			break;

		case 0x62: //not done
			break;

		case 0x63: //not done
			break;

		case 0x64: //not done
			break;

		case 0x65: //not done
			break;

		case 0x66: //not done
			break;

		case 0x67: //not done
			break;

		case 0x68: //not done
			break;

		case 0x69: //not done
			break;

		case 0x6a: //not done
			break;

		case 0x6b: //not done
			break;

		case 0x6c: //not done
			break;

		case 0x6d: //not done
			break;

		case 0x6e: //not done
			break;

		case 0x6f: //not done
			break;

		case 0x70: //not done
			break;

		case 0x71: //not done
			break;

		case 0x72: //not done
			break;

		case 0x73: //not done
			break;

		case 0x74: //not done
			break;

		case 0x75: //not done
			break;

		case 0x76: //not done
			break;

		case 0x77: //not done
			break;

		case 0x78: //not done
			break;

		case 0x79: //not done
			break;

		case 0x7a: //not done
			break;

		case 0x7b: //not done
			break;

		case 0x7c: //not done
			break;

		case 0x7d: //not done
			break;

		case 0x7e: //not done
			break;

		case 0x7f: //not done
			break;

		case 0x80: //not done
			break;

		case 0x81: //not done
			break;

		case 0x82: //not done
			break;

		case 0x83: //not done
			break;

		case 0x84: //not done
			break;

		case 0x85: //not done
			break;

		case 0x86: //not done
			break;

		case 0x87: //not done
			break;

		case 0x88: //not done
			break;

		case 0x89: //not done
			break;

		case 0x8a: //not done
			break;

		case 0x8b: //not done
			break;

		case 0x8c: //not done
			break;

		case 0x8d: //not done
			break;

		case 0x8e: //not done
			break;

		case 0x8f: //not done
			break;

		case 0x90: //not done
			break;

		case 0x91: //not done
			break;

		case 0x92: //not done
			break;

		case 0x93: //not done
			break;

		case 0x94: //not done
			break;

		case 0x95: //not done
			break;

		case 0x96: //not done
			break;

		case 0x97: //not done
			break;

		case 0x98: //not done
			break;

		case 0x99: //not done
			break;

		case 0x9a: //not done
			break;

		case 0x9b: //not done
			break;

		case 0x9c: //not done
			break;

		case 0x9d: //not done
			break;

		case 0x9e: //not done
			break;

		case 0x9f: //not done
			break;

		case 0xa0: //not done
			break;

		case 0xa1: //not done
			break;

		case 0xa2: //not done
			break;

		case 0xa3: //not done
			break;

		case 0xa4: //not done
			break;

		case 0xa5: //not done
			break;

		case 0xa6: //not done
			break;

		case 0xa7: //not done
			break;

		case 0xa8: //not done
			break;

		case 0xa9: //not done
			break;

		case 0xaa: //not done
			break;

		case 0xab: //not done
			break;

		case 0xac: //not done
			break;

		case 0xad: //not done
			break;

		case 0xae: //not done
			break;

		case 0xaf: //not done
			break;

		case 0xb0: //not done
			break;

		case 0xb1: //not done
			break;

		case 0xb2: //not done
			break;

		case 0xb3: //not done
			break;

		case 0xb4: //not done
			break;

		case 0xb5: //not done
			break;

		case 0xb6: //not done
			break;

		case 0xb7: //not done
			break;

		case 0xb8: //not done
			break;

		case 0xb9: //not done
			break;

		case 0xba: //not done
			break;

		case 0xbb: //not done
			break;

		case 0xbc: //not done
			break;

		case 0xbd: //not done
			break;

		case 0xbe: //not done
			break;

		case 0xbf: //not done
			break;

		case 0xc0: //not done
			break;

		case 0xc1: //not done
			break;

		case 0xc2: //not done
			break;

		case 0xc3: //not done
			break;

		case 0xc4: //not done
			break;

		case 0xc5: //not done
			break;

		case 0xc6: //not done
			break;

		case 0xc7: //not done
			break;

		case 0xc8: //not done
			break;

		case 0xc9: //not done
			break;

		case 0xca: //not done
			break;

		case 0xcc: //not done
			break;

		case 0xcd: //not done
			break;

		case 0xce: //not done
			break;

		case 0xcf: //not done
			break;

		case 0xd0: //not done
			break;

		case 0xd1: //not done
			break;

		case 0xd2: //not done
			break;

		case 0xd3: //not done
			break;

		case 0xd4: //not done
			break;

		case 0xd5: //not done
			break;

		case 0xd6: //not done
			break;

		case 0xd7: //not done
			break;

		case 0xd8: //not done
			break;

		case 0xda: //not done
			break;

		case 0xdb: //not done
			break;

		case 0xdc: //not done
			break;

		case 0xde: //not done
			break;

		case 0xdf: //not done
			break;

		case 0xe0: //not done
			break;

		case 0xe1: //not done
			break;

		case 0xe2: //not done
			break;

		case 0xe3: //not done
			break;

		case 0xe4: //not done
			break;

		case 0xe5: //not done
			break;

		case 0xe6: //not done
			break;

		case 0xe7: //not done
			break;

		case 0xe8: //not done
			break;

		case 0xe9: //not done
			break;

		case 0xea: //not done
			break;

		case 0xeb: //not done
			break;

		case 0xec: //not done
			break;

		case 0xee: //not done
			break;

		case 0xef: //not done
			break;

		case 0xf0: //not done
			break;

		case 0xf1: //not done
			break;

		case 0xf2: //not done
			break;

		case 0xf3: //not done
			break;

		case 0xf4: //not done
			break;

		case 0xf5: //not done
			break;

		case 0xf6: //not done
			break;

		case 0xf7: //not done
			break;

		case 0xf8: //not done
			break;

		case 0xf9: //not done
			break;

		case 0xfa: //not done
			break;

		case 0xfb: //not done
			break;

		case 0xfc: //not done
			break;

		case 0xfe: //not done
			break;

		case 0xff: //not done
			break;

		default:
			cout << "Unknown opcode: 0x" + *opcode << endl;  				
			break;
	}	
	pc++;
 
}

//flags - 0xf:ac,s,z,p  0x10:cy  0x1f:cy,ac,s,z,p
uint16_t si8080::setCond(uint32_t ans, uint16_t diff, uint16_t old, uint8_t flags) {
	if(flags & 0x10 == 0x10)
		cy = (ans > 0xff);
	if(flags & 0xf == 0xf) {
		ac = checkAC(ans & 0xff, diff, old);
		s = 0; //not done
		z = ((ans & 0xff) == 0);
		p = checkParity((uint8_t) ans & 0xff);
	}

	return ans;
}

uint8_t si8080::checkParity(uint8_t ans) {
	bitset<8> bs(ans);
	int count = 0;
	for(int i = 0; bs.to_string().size(); i++) {
		if(bs.to_string().substr(i, 1).compare("1") == 0) 
			count++;
	}

	return (count % 2 == 1);
}

uint8_t si8080::checkAC(uint8_t ans, uint16_t diff, uint16_t old) {
	if(diff > 0) {
		return (~((old ^ diff) & 0x10) == (ans & 0x10));
	}
	else if(diff < 0) {
		return ((old & 0x8) == 0 && ((diff * -1) & 0x8) == 1);
	}
	else {
		return -1; //shrug
	}
}

bool si8080::load(const char* filename) {
	cout << "Loading: " + (string)filename << endl;
		
	FILE* pFile = fopen(filename, "rb");
	if (pFile == NULL) {
		fputs("File error", stderr); 
		return false;
	}

	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	cout << "Filesize: " + (int)lSize << endl;
	
	char* buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL) {
		fputs("Memory error", stderr); 
		return false;
	}

	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize) {
		fputs("Reading error", stderr); 
		return false;
	}

	/*if((4096 - 512) > lSize) {
		for(int i = 0; i < lSize; i++)
			memory[i + 512] = buffer[i];
	}
	else
		cout << "Error: ROM too big for memory" << endl;*/
	
	fclose(pFile);
	free(buffer);

	return true;
}