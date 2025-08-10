#include "dram.h"

#pragma bss-name(push, "ZEROPAGE")
static u32 buffer;

u32* __fastcall__ drmLoad(struct DRAM* dram, unsigned int address) {
    if (address < DRAM_SIZE) {
        buffer.b[0] = dram->mem[address];
        buffer.b[1] = dram->mem[address + 1];
        buffer.b[2] = dram->mem[address + 2];
        buffer.b[3] = dram->mem[address + 3];
        
        return &buffer;
    }
    return 0; // Return zero if address is out of bounds
}

void __fastcall__ drmStore(struct DRAM* dram, unsigned int address, u32* value) {
    if (address < DRAM_SIZE - 3) {
        dram->mem[address] = value->b[0];
        dram->mem[address + 1] = value->b[1];
        dram->mem[address + 2] = value->b[2];
        dram->mem[address + 3] = value->b[3];
    }
}