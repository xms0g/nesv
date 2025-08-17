#include <string.h>
#include "neslib.h"
#include "config.h"
#include "riscv.h"
#include "machineCodes.h"

#pragma bss-name(push, "ZEROPAGE")
static u32* instr;
#pragma bss-name(pop)
static struct RiscV cpu;

void main(void) {
	ppu_off(); // screen off

	pal_bg(palette); //	load the BG palette

	rvInit(&cpu);

	memcpy(&cpu.bus.dram.mem, sh, sizeof(sh));
	
	while (1) { 
        instr = rvFetch(&cpu);

		if (instr->v == 0) break;

        rvDecode(&cpu, instr);

        rvExecute(&cpu);

		cpu.pc += 4;
        
    }
	
	ppu_on_all(); // turn on screen1
	
	
	while (1){}
}