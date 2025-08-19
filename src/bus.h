#ifndef BUS_H
#define BUS_H  

#include "dram.h"

struct Bus {
    struct DRAM dram;
};

unsigned long __fastcall__ busLoad(const struct Bus* bus, unsigned long address, unsigned char size);

void __fastcall__ busStore(struct Bus* bus, unsigned long address, unsigned char size, unsigned long value);

#endif