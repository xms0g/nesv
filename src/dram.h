#ifndef DRAM_H
#define DRAM_H

#define DRAM_SIZE 1024 
#define DRAM_BASE 0x80000000

struct DRAM {
    unsigned char mem[DRAM_SIZE];
};

unsigned long __fastcall__ drmLoad(struct DRAM* dram, unsigned long address, unsigned char size);

void __fastcall__ drmStore(struct DRAM* dram, unsigned long address, unsigned char size, unsigned long value);

#endif