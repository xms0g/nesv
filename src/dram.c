#include "dram.h"

#pragma bss-name(push, "ZEROPAGE")
static u32 buffer;
unsigned long offset;
#pragma bss-name(pop)

u32* __fastcall__ drmLoad(struct DRAM* dram, unsigned long address) {
    if (address >= DRAM_BASE && address <= DRAM_BASE + DRAM_SIZE - 4) {
        offset = address - DRAM_BASE;
        buffer.b[0] = dram->mem[offset];
        buffer.b[1] = dram->mem[offset + 1];
        buffer.b[2] = dram->mem[offset + 2];
        buffer.b[3] = dram->mem[offset + 3];
        
        return &buffer;
    }
    
    return 0;
}

void __fastcall__ drmStore(struct DRAM* dram, unsigned long address, u32* value) {
    if (address >= DRAM_BASE && address <= DRAM_BASE + DRAM_SIZE - 4) {
        offset = address - DRAM_BASE;
        dram->mem[offset] = value->b[0];
        dram->mem[offset + 1] = value->b[1];
        dram->mem[offset + 2] = value->b[2];
        dram->mem[offset + 3] = value->b[3];
    }
}