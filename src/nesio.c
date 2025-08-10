#include "nesio.h"
#include <stdlib.h>
#include "neslib.h"

#pragma bss-name(push, "ZEROPAGE")
static unsigned char i;
static char buf[6];

void __fastcall__ print(const unsigned char* str) {
    for (i = 0; str[i]; ++i) {
        vram_put(str[i]);
    }
}

void __fastcall__ printReg(const unsigned char reg, const u32* val) {
    unsigned int value = val->b[0] | ((unsigned int)val->b[1] << 8);
    
    vram_put('x');
    vram_put(reg + '0');
    vram_put(':');
    
    utoa(value, buf, 10);

    // Print each digit
    print(buf);
}

void __fastcall__ printImm(const u32* val) {
    unsigned int value = val->b[0] | ((unsigned int)val->b[1] << 8);
    
    vram_put('i');
    vram_put('m');
    vram_put('m');
    vram_put(':');
    
    utoa(value, buf, 10);
    
    print(buf);
}
