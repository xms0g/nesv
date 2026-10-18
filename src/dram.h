#ifndef DRAM_H
#define DRAM_H

#define DRAM_SIZE 1024 
#define DRAM_BASE 0x80000000

struct DRAM {
    unsigned char mem[DRAM_SIZE];
};

unsigned long* __fastcall__ drm_load(const struct DRAM* dram, const unsigned long* address, unsigned char size);

void __fastcall__ drm_store(struct DRAM* dram, const unsigned long* address, unsigned char size, const unsigned long* value);

#endif