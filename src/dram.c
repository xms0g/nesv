#include "dram.h"

#pragma bss-name(push, "ZEROPAGE")
static u32 buffer;
#pragma bss-name(pop)

u32* __fastcall__ drmLoad(struct DRAM* dram, unsigned long address) {
    if (address >= DRAM_BASE && address <= DRAM_BASE + DRAM_SIZE - 4) {
        buffer.b[0] = dram->mem[address - DRAM_BASE];
        buffer.b[1] = dram->mem[address - DRAM_BASE + 1];
        buffer.b[2] = dram->mem[address - DRAM_BASE + 2];
        buffer.b[3] = dram->mem[address - DRAM_BASE + 3];
        
        return &buffer;
    }
    
    return 0;
}

void __fastcall__ drmStore(struct DRAM* dram, unsigned long address, u32* value) {
    if (address < DRAM_SIZE - 3) {
        dram->mem[address] = value->b[0];
        dram->mem[address + 1] = value->b[1];
        dram->mem[address + 2] = value->b[2];
        dram->mem[address + 3] = value->b[3];
    }
}