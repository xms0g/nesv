#include "dram.h"
#include <string.h>

#pragma bss-name(push, "ZEROPAGE")
static unsigned long data;
static unsigned long offset;
#pragma bss-name(pop)

unsigned long* __fastcall__ drmLoad(const struct DRAM* dram, const unsigned long* address, unsigned char size) {
    if (*address >= DRAM_BASE && *address < DRAM_BASE + DRAM_SIZE) {
        offset = *address - DRAM_BASE;

        switch (size) {
            case 8:
                data = (unsigned long)dram->mem[offset];
                break;
            case 16:
                data = (unsigned long)dram->mem[offset] | 
                        (unsigned long)dram->mem[offset + 1] << 8;
                break;
            case 32:
                data = (unsigned long)dram->mem[offset] | 
                        (unsigned long)dram->mem[offset + 1] << 8  |
                        (unsigned long)dram->mem[offset + 2] << 16 |
                        (unsigned long)dram->mem[offset + 3] << 24;
                break;
        }
        return &data;
    }
    return 0;
}

void __fastcall__ drmStore(struct DRAM* dram, const unsigned long* address, unsigned char size, const unsigned long* value) {
    if (*address >= DRAM_BASE && *address < DRAM_BASE + DRAM_SIZE) {
        offset = *address - DRAM_BASE;

        switch (size) {
            case 8:
                dram->mem[offset] = (unsigned char)(*value & 0xFF);
                break;
            case 16:
                dram->mem[offset] = (unsigned char)(*value & 0xFF);
                dram->mem[offset + 1] = (unsigned char)((*value >> 8) & 0xFF);
                break;
            case 32:
                dram->mem[offset] = (unsigned char)(*value & 0xFF);
                dram->mem[offset + 1] = (unsigned char)((*value >> 8) & 0xFF);
                dram->mem[offset + 2] = (unsigned char)((*value >> 16) & 0xFF);
                dram->mem[offset + 3] = (unsigned char)((*value >> 24) & 0xFF);
                break;
        }
    }
}