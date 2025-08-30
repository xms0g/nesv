#include "dram.h"
#include <string.h>

#pragma bss-name(push, "ZEROPAGE")
unsigned long data;
unsigned char* offset;
#pragma bss-name(pop)

unsigned long* __fastcall__ drmLoad(const struct DRAM* dram, const unsigned long* address, unsigned char size) {
    if (*address >= DRAM_BASE && *address < DRAM_BASE + DRAM_SIZE) {
        offset = (unsigned char*)&dram->mem[*address - DRAM_BASE];

        switch (size) {
            case 8:
                data = *offset;
                break;
            case 16:
                data = (unsigned long)*offset++;
                data |= (unsigned long)*offset << 8;
                break;
            case 32:
                data = (unsigned long)*offset++;
                data |= (unsigned long)*offset++ << 8;
                data |= (unsigned long)*offset++ << 16;
                data |= (unsigned long)*offset << 24;
                break;
        }
        return &data;
    }
    return 0;
}

void __fastcall__ drmStore(struct DRAM* dram, const unsigned long* address, unsigned char size, const unsigned long* value) {
    if (*address >= DRAM_BASE && *address < DRAM_BASE + DRAM_SIZE) {
        offset = &dram->mem[*address - DRAM_BASE];

        switch (size) {
            case 8:
                *offset = (unsigned char)(*value & 0xFF);
                break;
            case 16:
                *offset++ = (unsigned char)(*value & 0xFF);
                *offset = (unsigned char)((*value >> 8) & 0xFF);
                break;
            case 32:
                *offset++ = (unsigned char)(*value & 0xFF);
                *offset++ = (unsigned char)((*value >> 8) & 0xFF);
                *offset++ = (unsigned char)((*value >> 16) & 0xFF);
                *offset = (unsigned char)((*value >> 24) & 0xFF);
                break;
        }
    }
}