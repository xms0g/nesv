#ifndef NESIO_H
#define NESIO_H

#include "../libs/neslib.h"

#define NEXT_CHAR(n) do { x += (n); vram_adr(NTADR_A(x, y)); } while(0)
#define NEXT_LINE(n) do { x = n; ++y; vram_adr(NTADR_A(x, y)); } while(0)
#define SETXY(_x, _y) do { x = _x; y = _y; vram_adr(NTADR_A(x, y)); } while(0)

#define PUT(str) do { \
    print(str); \
    NEXT_CHAR(6); \
    } while(0)

#define PUTR(r) do { \
    print("x"); \
    uprint(r); \
    NEXT_CHAR(4); \
    } while(0)

#define PUTSI(n) do { \
    sprint(n); \
    NEXT_CHAR(4); \
    } while(0)

#define PUTUI(v) do { \
    ulprint(v); \
    NEXT_CHAR(4); \
    } while(0)

#define PUTLS(imm, rs1) do { \
    sprint(imm); \
    print("(");print("x");uprint(rs1);print(")"); \
    } while(0)

void __fastcall__ print(const unsigned char* str);

void __fastcall__ sprint(long val);

void __fastcall__ uprint(unsigned char val);

void __fastcall__ ulprint(unsigned long val);

void __fastcall__ xprint(unsigned long val);

#endif