#include "bus.h"

u32* __fastcall__ busLoad(struct Bus* bus, unsigned long address) {
    return drmLoad(&bus->dram, address);
}

void __fastcall__ busStore(struct Bus* bus, unsigned long address, u32* value) {
    drmStore(&bus->dram, address, value);
}