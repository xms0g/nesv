#include "nesio.h"
#include <stdlib.h>

#pragma bss-name(push, "ZEROPAGE")
static char buf[5];
#pragma bss-name(pop)

void __fastcall__ print(const unsigned char* str) {
    while (*str) {
        vram_put(*str++);
    }
}

void __fastcall__ printReg(const unsigned char reg, const long val) {
    vram_put('x');
    utoa(reg, buf, 10);
    print(buf);
  
    vram_put(':');
    
    itoa(val, buf, 10);
    print(buf);
}

void __fastcall__ printImm(const long val) {
    vram_put('i');
    vram_put(':');
    
    itoa(val, buf, 10);
    print(buf);
}