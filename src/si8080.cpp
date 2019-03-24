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
		case 0x00:
			switch(*opcode & 0x0F) {
				case 0x00: //NOP
					break;

				case 0x01: //LXI B,D16
					c = opcode[1];
					b = opcode[2];	
					pc += 2;	
					break;

				case 0x02: //STAX B B = 3f c = 16 A -> memory[3f16]
					memory[(b << 8) + c] = &a;
					break;
					
				case 0x03: //INX B b = 3f and c = 16
				{
					uint16_t tmp = ((uint16_t) b << 8) + c + 1;
					c = (tmp & 0xff);
					b = (tmp >> 8);
				}
					break;
					
				case 0x04: //INR B
					b = checkCond((uint16_t) b + 1);
					break;
					
				case 0x05: //3 bytes 1: the instruction 2: the "low data" 3: the "high data"
					c = opcode[1];
					b = opcode[2];	
					pc += 2;	
					break;

				default:
					cout << "Unknown opcode [0x00]: 0x" + *opcode << endl; 					
					break;					
			}
			break;

		default:
			cout << "Unknown opcode: 0x" + *opcode << endl;  				
			break;
	}	
	pc++;
 
}

uint8_t si8080::checkCond(uint16_t ans) {
	cy =
	s =
	z = ((ans & 0xff) == 0);
	p = checkParity(ans & 0xff);
	return (ans & 0xff);
}

uint8_t si8080::checkParity(uint16_t ans) {
	bitset<16> bs(ans);
	int count = 0;
	for(int i = 0; bs.to_string().size(); i++) {
		if(bs.to_string().substr(i, 1).compare("1") == 0) 
			count++;
	}
	return count % 2;
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