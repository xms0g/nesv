#include "neslib.h"
#include "config.h"
#include "riscv.h" 



// li t1, 1
// li t2, 2
// add t0, t1, t2 
static const unsigned char code[] = {
	 // li t1, 1     → addi t1, x0, 1     → 0x00100313
    0x13, 0x03, 0x10, 0x00,

    // li t2, 2     → addi t2, x0, 2     → 0x00200393
    0x93, 0x03, 0x20, 0x00,

    // add t0, t1, t2 → add x5, x6, x7   → 0x007302b3
    0xb3, 0x02, 0x73, 0x00
};

static struct RiscV cpu;
static u32 instr;

void main(void) {
	ppu_off(); // screen off

	pal_bg(palette); //	load the BG palette

	rvInit(&cpu);
	
	while (cpu.pc < sizeof(code)) { 
        //vFetch();
		unsigned char* p = (unsigned char*)(code + cpu.pc);
		instr.b[0] = p[0];
		instr.b[1] = p[1];
		instr.b[2] = p[2];
		instr.b[3] = p[3];
        rvDecode(&cpu, &instr);

        rvExecute(&cpu);

		cpu.pc += 4;
        
    }
	
	// vram_adr and vram_put only work with screen off
	// NOTE, you could replace everything between i = 0; and here with...
	// vram_write(text,sizeof(text));
	// does the same thing
	
	ppu_on_all(); //	turn on screen1
	
	
	while (1){
		// infinite loop
		// game code can go here later.	
	}
}