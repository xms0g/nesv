#include "nesio.h"
#include <stdlib.h>

#pragma bss-name(push, "ZEROPAGE")
static char buf[12];
#pragma bss-name(pop)

void __fastcall__ print(const unsigned char* str) {
    while (*str) {
        vram_put(*str++);
    }
}

void __fastcall__ printReg(const unsigned char reg, const long val) {
    utoa(reg, buf, 10);
    
    print("x");
    print(buf);
    print(":");
    
    ltoa(val, buf, 10);
    print(buf);
}

void __fastcall__ printImm(const long val) {
    ltoa(val, buf, 10);
    print("i:");
    print(buf);
}