#ifndef NESIO_H
#define NESIO_H

#include <string.h>
#include "neslib.h"
#include "types.h"

#define NEXT_CHAR(n) do { \
    x += (n); \
    } while(0)

#define PUT(str) do { \
    vram_adr(NTADR_A(x, y)); \
    print(str); \
    NEXT_CHAR(strlen(str) + 1); \
    } while(0)

#define PUTR(r, v) do { \
    vram_adr(NTADR_A(x, y)); \
    printReg(r, v); \
    if (v & 0x800) NEXT_CHAR(5); \
    else NEXT_CHAR(4); \
    } while(0)

#define PUTI(v) do { \
    vram_adr(NTADR_A(x, y)); \
    printImm(v); \
    if (v & 0x800) NEXT_CHAR(5); \
    else NEXT_CHAR(4); \
    } while(0)

#define NEXT_LINE() do { \
    x = 1; \
    y++; \
    } while(0)

void __fastcall__ print(const unsigned char* str);

void __fastcall__ printReg(const unsigned char reg, const long val);

void __fastcall__ printImm(const long val);

#endif