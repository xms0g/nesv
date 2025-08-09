#ifndef RISCV_H
#define RISCV_H

#include "types.h"

struct RiscVInstr {
    unsigned char opcode;
    unsigned char rd;
    unsigned char funct3;
    unsigned char rs1;
    unsigned char rs2;
    unsigned char funct7;
    u32 imm;
};

struct RiscV {
    u32 regs[32];
    struct RiscVInstr instr;
    unsigned char pc;
};

void __fastcall__ rvInit(struct RiscV* cpu);

int __fastcall__ vFetch(void);
void __fastcall__ rvDecode(struct RiscV* cpu, const u32* raw);
void __fastcall__ rvExecute(struct RiscV* cpu);

#endif