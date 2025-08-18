#include <string.h>
#include "config.h"
#include "riscv.h"
#include "../libs/neslib.h"
#include "machineCodes.h"

#pragma bss-name(push, "ZEROPAGE")
static u32* instr;
unsigned char hasJump;
#pragma bss-name(pop)
static struct RiscV cpu;

void main(void) {
	ppu_off(); // screen off

	pal_bg(palette); //	load the BG palette

	rvInit(&cpu);

	hasJump = 0;

	memcpy(&cpu.bus.dram.mem, beq, sizeof(beq));
	
	while (1) { 
        instr = rvFetch(&cpu);

		if (instr->v == 0) break;

        rvDecode(&cpu, instr);

        rvExecute(&cpu, &hasJump);

		if (!hasJump)
			cpu.pc += 4;
		
		hasJump = 0;   
    }

	rvDumpReg(&cpu);
	
	ppu_on_all(); // turn on screen1
	
	
	while (1){}
}