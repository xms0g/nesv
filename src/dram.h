#ifndef DRAM_H
#define DRAM_H

#include "types.h"

#define DRAM_SIZE 1000 

struct DRAM {
    unsigned char mem[DRAM_SIZE];
};

u32* __fastcall__ drmLoad(struct DRAM* dram, unsigned int address);

void __fastcall__ drmStore(struct DRAM* dram, unsigned int address, u32* value);

#endif