#include <string.h>
#include "config.h"
#include "riscv.h"
#include "../libs/neslib.h"

#pragma bss-name(push, "ZEROPAGE")
unsigned long* instr;
unsigned char hasJump;
#pragma bss-name(pop)

struct RiscV cpu;

const unsigned char code[] = {
	// addi sp, sp, -8
    0x13, 0x01, 0x81, 0xff,
    // li	t1, -500
    0x13, 0x03, 0xc0, 0xe0,
	// sh   t1, 0(sp)
    0x23, 0x10, 0x61, 0x00,
	// lh   t2, 0(sp)
    0x83, 0x13, 0x01, 0x00,
    // sll 	t3, t2, 5
    0x13, 0x9e, 0x53, 0x00,
   	// addi sp, sp, 8
    0x13, 0x01, 0x81,0x00,
};

void main(void) {
	ppu_off(); // screen off

	pal_bg(palette); //	load the BG palette

	rvInit(&cpu);

	hasJump = 0;

	memcpy(&cpu.bus.dram.mem, code, sizeof(code));
	
	while (1) { 
        instr = rvFetch(&cpu);

		if (*instr == 0) break;

        rvDecode(&cpu, instr, &hasJump);

        rvExecute(&cpu);

		if (!hasJump)
			cpu.pc += 4;
		
		hasJump = 0;   
    }

	rvDumpReg(&cpu);
	
	ppu_on_all(); // turn on screen1
	
	while (1){}
}