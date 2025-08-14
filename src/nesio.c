#include "nesio.h"
#include <stdlib.h>


#pragma bss-name(push, "ZEROPAGE")
static unsigned char i;
static char buf[6];
#pragma bss-name(pop)

void __fastcall__ print(const unsigned char* str) {
    for (i = 0; str[i]; ++i) {
        vram_put(str[i]);
    }
}

void __fastcall__ printReg(const unsigned char reg, const long val) {
    vram_put('x');
    utoa(reg, buf, 10);
    print(buf);
    
    vram_put(':');
    
    itoa(val, buf, 10);
    print(buf);
    memset(buf, 0, 6);
}

void __fastcall__ printImm(const long val) {
    vram_put('i');
    vram_put('m');
    vram_put('m');
    vram_put(':');
    
    itoa(val, buf, 10);
    print(buf);
    memset(buf, 0, 6);
}
