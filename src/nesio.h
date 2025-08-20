#ifndef NESIO_H
#define NESIO_H

#include "../libs/neslib.h"

#define NEXT_CHAR(n) do { x += (n); vram_adr(NTADR_A(x, y)); } while(0)
#define NEXT_LINE(n) do { x = n; ++y; vram_adr(NTADR_A(x, y)); } while(0)
#define GOTOXY(n0, n1) do { x = n0; y = n1; vram_adr(NTADR_A(x, y)); } while(0)

#define PUT(str) do { \
    print(str); \
    NEXT_CHAR(6); \
    } while(0)

#define PUTR(r) do { \
    printXReg(r); \
    NEXT_CHAR(4); \
    } while(0)

#define PUTSI(n) do { \
    sprint(n); \
    NEXT_CHAR(4); \
    } while(0)

#define PUTUI(v) do { \
    uprint(v); \
    NEXT_CHAR(4); \
    } while(0)

#define PUTLS(imm, rs1) do { \
    sprint(imm); \
    print("(");printXReg(rs1);print(")"); \
    } while(0)

void __fastcall__ print(const unsigned char* str);

void __fastcall__ sprint(const long val);

void __fastcall__ uprint(const unsigned long val);

void __fastcall__ xprint(const unsigned long val);

void __fastcall__ printXReg(const unsigned char reg);

#endif