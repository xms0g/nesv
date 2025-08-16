#ifndef NESIO_H
#define NESIO_H

#include <string.h>
#include "neslib.h"
#include "types.h"

#define NEXT_CHAR(n) do { x += (n); vram_adr(NTADR_A(x, y)); } while(0)

#define PUT(str) do { \
    print(str); \
    NEXT_CHAR(strlen(str) + 1); \
    } while(0)

#define PUTR(r, v) do { \
    printReg(r, v); \
    NEXT_CHAR(12); \
    } while(0)

#define PUTSI(v) do { \
    print("i:"); \
    sprint(v); \
    NEXT_CHAR(12); \
    } while(0)

#define PUTUI(v) do { \
    print("i:"); \
    uprint(v); \
    NEXT_CHAR(12); \
    } while(0)

#define NEXT_LINE() do { x = 1; ++y; vram_adr(NTADR_A(x, y)); } while(0)

void __fastcall__ print(const unsigned char* str);

void __fastcall__ sprint(const long val);

void __fastcall__ uprint(const unsigned long val);

void __fastcall__ printReg(const unsigned char reg, const long val);

#endif