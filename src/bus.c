#include "bus.h"

unsigned long* __fastcall__ busLoad(const struct Bus* bus, const unsigned long* address, unsigned char size) {
    return drmLoad(&bus->dram, address, size);
}

void __fastcall__ busStore(struct Bus* bus, const unsigned long* address, unsigned char size, const unsigned long* value) {
    drmStore(&bus->dram, address, size, value);
}