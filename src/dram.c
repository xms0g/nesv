#include "dram.h"
#include <string.h>

#pragma bss-name(push, "ZEROPAGE")
static u32 buffer;
unsigned long offset;
#pragma bss-name(pop)

u32* __fastcall__ drmLoad(struct DRAM* dram, unsigned long address, unsigned char size) {
    if (address >= DRAM_BASE && address < DRAM_BASE + DRAM_SIZE) {
        offset = address - DRAM_BASE;

        switch (size) {
            case 8:
                buffer.b[0] = dram->mem[offset];
                break;
            case 16:
                buffer.b[0] = dram->mem[offset];
                buffer.b[1] = dram->mem[offset + 1];
                break;
            case 32:
                buffer.b[0] = dram->mem[offset];
                buffer.b[1] = dram->mem[offset + 1];
                buffer.b[2] = dram->mem[offset + 2];
                buffer.b[3] = dram->mem[offset + 3];
                break;
        }

        return &buffer;
    }
    return 0;
}

void __fastcall__ drmStore(struct DRAM* dram, unsigned long address, unsigned char size, u32* value) {
    if (address >= DRAM_BASE && address < DRAM_BASE + DRAM_SIZE) {
        offset = address - DRAM_BASE;

        switch (size) {
            case 8:
                dram->mem[offset] = value->b[0];
                break;
            case 16:
                dram->mem[offset] = value->b[0];
                dram->mem[offset + 1] = value->b[1];
                break;
            case 32:
                dram->mem[offset] = value->b[0];
                dram->mem[offset + 1] = value->b[1];
                dram->mem[offset + 2] = value->b[2];
                dram->mem[offset + 3] = value->b[3];
                break;
        }
    }
}