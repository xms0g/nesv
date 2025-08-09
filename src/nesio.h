#ifndef NESIO_H
#define NESIO_H

#include "types.h"

void __fastcall__ printOP(const unsigned char* op, unsigned char* x, unsigned char* y);

void __fastcall__ printREG(const unsigned char reg, const u32* val, unsigned char* x, unsigned char* y);

#endif