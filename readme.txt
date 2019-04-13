/***********************************/

         si8080Test

/***********************************/
DESCRIPTION: Test the conditional jumps, the mathematical and logical operations of the 8080 for use of a space invaders emulator
AUTHOR: Pixel
CONTACT: discord: Pixel#4370
COPYRIGHT: I don't care lmao
VERSION: 4/13/2019

/***********************************/

    HOW TO USE IT:

/***********************************/
Each error is accompanied by a number that identifies the opcode in question. If all tests are positive, the rom message will display on screen.
Note: you must have call instruction already working

/***********************************/

    CORRESPONDENCE

/***********************************/
Starting conditions: Z: 0   C: 0    P:  0
E 01: 0xC2 verify the jump not zero is fair
E 02: 0xC4 verify the call not zero is fair
E 03: 0xCA verify the jump zero is fair
E 04: 0xCC verify the call zero is fair
E 05: 0xD2 verify the jump not carry is fair
E 06: 0xD4 verify the call not carry is fair
E 07: 0xDA verify the jump carry is fair
E 08: 0xDC verify the call carry is fair
E 09: 8XY1 check the result of the OR operation
E 10: 8XY2 check the result of AND operation
E 11: 8XY3 check the result of the XOR operation
E 12: 8XYE verify that VF is set to the MSB (most significant bit or most left) before the shift and  VF does not take value 0 every time
E 13: 8XYE verify that VF is set to the MSB (most significant bit or most left) before the shift and  VF does not take value 1 every time 
E 14: 8XY6 verify that VF is set to the LSB (least significant bit or most right ) before the shift and  VF does not take value 0 every time
E 15: 8XY6 verify that VF is the LSB (least significant bit or most right) before the shift and  VF does not take value 1 every time 
E 16: FX55 and FX65 verify that these two opcodes are implemented. The error may come from one or the other or both are defects.
E 17: FX33 calculating the binary representation is mistaken or the result is poorly stored into memory or poorly poped (FX65 or FX1E).

/**************************/

    Happy debugging

/**************************/

(oh btw I totally didn't steal this text file from bestcoder omegalul)