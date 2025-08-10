#ifndef NESIO_H
#define NESIO_H

#include "types.h"

void __fastcall__ print(const unsigned char* str);

void __fastcall__ printReg(const unsigned char reg, const u32* val);

void __fastcall__ printImm(const u32* val);

#endif