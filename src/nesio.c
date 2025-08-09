#include "nesio.h"
#include "neslib.h"

void __fastcall__ printOP(const unsigned char* op, unsigned char* x, unsigned char* y) {
    unsigned char i = 0;
    
    vram_adr(NTADR_A(*x, ++*y));
    while (op[i]) {
        vram_put(op[i]);
        ++i;
    }
}

void __fastcall__ printREG(const unsigned char reg, const u32* val, unsigned char* x, unsigned char* y) {
    vram_adr(NTADR_A(*x, ++*y));
    
    if (reg == 'i') {
        vram_put('i');  
    } else {
        vram_put('x');
        vram_put(reg + '0');
    }

    vram_put(':');
    vram_put(val->b[0] + '0');
}
