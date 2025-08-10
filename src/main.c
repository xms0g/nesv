#include "neslib.h"
#include "config.h"
#include "riscv.h"
#include "machineCodes.h"

#pragma bss-name(push, "ZEROPAGE")
static u32* instr;

#pragma bss-name(push, "BSS")
static struct RiscV cpu;

void main(void) {
	ppu_off(); // screen off

	pal_bg(palette); //	load the BG palette

	rvInit(&cpu);

	memcpy(&cpu.bus.dram.mem, sll, sizeof(sll));
	
	while (1) { 
        instr = rvFetch(&cpu);

		if (instr->b[0] == 0) break;

        rvDecode(&cpu, instr);

        rvExecute(&cpu);

		cpu.pc += 4;
        
    }
	
	ppu_on_all(); // turn on screen1
	
	
	while (1){}
}