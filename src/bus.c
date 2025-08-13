#include "bus.h"

u32* __fastcall__ busLoad(struct Bus* bus, unsigned long address, unsigned char size) {
    return drmLoad(&bus->dram, address, size);
}

void __fastcall__ busStore(struct Bus* bus, unsigned long address, unsigned char size, u32* value) {
    drmStore(&bus->dram, address, size, value);
}