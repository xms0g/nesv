#ifndef BUS_H
#define BUS_H  

#include "types.h"
#include "dram.h"

struct Bus {
    struct DRAM dram;
};

u32* __fastcall__ busLoad(struct Bus* bus, unsigned int address);

void __fastcall__ busStore(struct Bus* bus, unsigned int address, u32* value);

#endif