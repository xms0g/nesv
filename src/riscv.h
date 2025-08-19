#ifndef RISCV_H
#define RISCV_H

#include "bus.h"

#define X0  R_ZERO
#define X1  R_RA
#define X2  R_SP
#define X3  R_GP
#define X4  R_TP
#define X5  R_T0
#define X6  R_T1
#define X7  R_T2
#define X8  R_S0
#define X9  R_S1
#define X10 R_A0
#define X11 R_A1
#define X12 R_A2
#define X13 R_A3
#define X14 R_A4
#define X15 R_A5
#define X16 R_A6
#define X17 R_A7
#define X18 R_S2
#define X19 R_S3
#define X20 R_S4
#define X21 R_S5
#define X22 R_S6
#define X23 R_S7
#define X24 R_S8
#define X25 R_S9
#define X26 R_S10
#define X27 R_S11
#define X28 R_T3
#define X29 R_T4
#define X30 R_T5
#define X31 R_T6

enum Registers {
    R_ZERO = 0,
    R_RA = 1,
    R_SP = 2,
    R_GP = 3,
    R_TP = 4,
    R_T0 = 5,
    R_T1 = 6,
    R_T2 = 7,
    R_S0 = 8,
    R_FP = 8,
    R_S1 = 9,
    R_A0 = 10,
    R_A1 = 11,
    R_A2 = 12,
    R_A3 = 13,
    R_A4 = 14,
    R_A5 = 15,
    R_A6 = 16,
    R_A7 = 17,
    R_S2 = 18,
    R_S3 = 19,
    R_S4 = 20,
    R_S5 = 21,
    R_S6 = 22,
    R_S7 = 23,
    R_S8 = 24,
    R_S9 = 25,
    R_S10 = 26,
    R_S11 = 27,
    R_T3 = 28,
    R_T4 = 29,
    R_T5 = 30,
    R_T6 = 31
};

struct RiscVInstr {
    unsigned char opcode;
    unsigned char rd;
    unsigned char funct3;
    unsigned char rs1;
    unsigned char rs2;
    unsigned char funct7;
    long imm;
};

struct RiscV {
    unsigned long regs[32];
    struct RiscVInstr instr;
    unsigned long pc;
    struct Bus bus;
};

void __fastcall__ rvInit(struct RiscV* cpu);

unsigned long __fastcall__ rvFetch(struct RiscV* cpu);

void __fastcall__ rvDecode(struct RiscV* cpu, const unsigned long raw, unsigned char* hasJump);

void __fastcall__ rvExecute(struct RiscV* cpu);

void __fastcall__ rvDumpReg(struct RiscV* cpu);

#endif