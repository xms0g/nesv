#ifndef BUS_H
#define BUS_H  

#include "dram.h"

struct Bus {
    struct DRAM dram;
};

unsigned long* __fastcall__ busLoad(const struct Bus* bus, const unsigned long* address, unsigned char size);

void __fastcall__ busStore(struct Bus* bus, const unsigned long* address, unsigned char size, const unsigned long* value);

#endif