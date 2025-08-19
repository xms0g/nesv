#include "bus.h"

unsigned long __fastcall__ busLoad(struct Bus* bus, unsigned long address, unsigned char size) {
    return drmLoad(&bus->dram, address, size);
}

void __fastcall__ busStore(struct Bus* bus, unsigned long address, unsigned char size, unsigned long value) {
    drmStore(&bus->dram, address, size, value);
}