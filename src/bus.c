#include "bus.h"

unsigned long* __fastcall__ bus_load(const struct Bus* bus, const unsigned long* address, unsigned char size) {
    return drm_load(&bus->dram, address, size);
}

void __fastcall__ bus_store(struct Bus* bus, const unsigned long* address, unsigned char size, const unsigned long* value) {
    drm_store(&bus->dram, address, size, value);
}