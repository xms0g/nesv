#ifndef BUS_H
#define BUS_H  

#include "types.h"
#include "dram.h"

struct Bus {
    struct DRAM dram;
};

u32* __fastcall__ busLoad(struct Bus* bus, unsigned long address, unsigned char size);

void __fastcall__ busStore(struct Bus* bus, unsigned long address, u32* value);

#endif