#include "riscv.h"
#include "neslib.h"
#include "nesio.h"

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

static unsigned char x = 1;
static unsigned char y = 0;

#pragma bss-name(push, "ZEROPAGE")
static unsigned int i;
static unsigned int carry;
static unsigned char newCarry;
static unsigned int borrow;
static unsigned char shift;
static u32 imm4;
#pragma bss-name(pop)

/* R-type Instructions */
static void __fastcall__ subU32fromU32(u32* dst, const u32* src);
static void __fastcall__ xorU32withU32(u32* dst, const u32* src);
static void __fastcall__ orU32withU32(u32* dst, const u32* src);
static void __fastcall__ andU32withU32(u32* dst, const u32* src);
static void __fastcall__ sllU32withU32(u32* dst, const u32* src);
static void __fastcall__ srlU32withU32(u32* dst, const u32* src);
static void __fastcall__ sraU32withU32(u32* dst, const u32* src);
static void __fastcall__ sltU32withU32(u32* dst, const u32* src);
static void __fastcall__ sltuU32withU32(u32* dst, const u32* src);

/* I-type Instructions */
static void __fastcall__ xorImm16withU32(u32* dst, const unsigned char imm_bytes[2]);
static void __fastcall__ orImm16withU32(u32* dst, const unsigned char imm_bytes[2]);
static void __fastcall__ andImm16withU32(u32* dst, const unsigned char imm_bytes[2]);
static void __fastcall__ sllImm16withU32(u32* dst, const unsigned char imm_bytes[2]);
static void __fastcall__ srlImm16withU32(u32* dst, const unsigned char imm_bytes[2]);
static void __fastcall__ sraImm16withU32(u32* dst, const unsigned char imm_bytes[2]);
static void __fastcall__ sltImm16withU32(u32* dst, const unsigned char imm_bytes[2]);
static void __fastcall__ sltuImm16withU32(u32* dst, const unsigned char imm_bytes[2]);

static void __fastcall__ makeImm4fromImm2(const unsigned char imm[2])  {
    imm4.b[0] = imm[0];
    imm4.b[1] = imm[1];

    if (imm[1] & 0x80) {
        imm4.b[2] = 0xFF;
        imm4.b[3] = 0xFF;
    } else {
        imm4.b[2] = 0x00;
        imm4.b[3] = 0x00;
    }
}

void __fastcall__ rvInit(struct RiscV* cpu) {
    memset(cpu->regs, 0, sizeof(u32) * 32);
    
    cpu->regs[X0].b[0] = 0x0;
    // Set R_SP = 0x80000400 (example stack start)
    cpu->regs[R_SP].b[0] = 0x00;       // lowest byte
    cpu->regs[R_SP].b[1] = 0x04;
    cpu->regs[R_SP].b[2] = 0x00;
    cpu->regs[R_SP].b[3] = 0x80;       // highest byte
    cpu->pc = DRAM_BASE;
}

u32* __fastcall__ rvFetch(struct RiscV* cpu) {
    return busLoad(&cpu->bus, cpu->pc, 32);
}

void __fastcall__ rvDecode(struct RiscV* cpu, const u32* raw) {
    cpu->instr.opcode = raw->v & 0x7F;
    cpu->instr.rd = ((raw->v >> 7) & 0x1f);
    cpu->instr.funct3 = (raw->v >> 12) & 0x07;
    cpu->instr.rs1 = (raw->v >> 15) & 0x1f;
   
    switch (cpu->instr.opcode) {
        case 0x33: // R-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: // add/sub
                case 0x4: // xor
                case 0x6: // or
                case 0x7: // and
                case 0x1: // sll
                case 0x5: // srl/sra
                case 0x2: // slt
                case 0x3: // sltu
                    cpu->instr.rs2 = (raw->v >> 20) & 0x1f;
                    cpu->instr.funct7 = (raw->v >> 25) & 0x7F;
                    break;
            }
            break;
        case 0x13: // I-type Instructions
        case 0x3: // Load Instructions
            switch (cpu->instr.funct3) {
                case 0x0: // addi
                case 0x4: // xori
                case 0x6: // ori
                case 0x7: // andi
                case 0x1: // slli
                case 0x5: // srli/srai
                case 0x2: // slti
                case 0x3: { // sltiu
                    cpu->instr.imm.v = (raw->v >> 20) & 0xFFF;
                    
                    if (cpu->instr.imm.v & 0x800)
                        cpu->instr.imm.v |= 0xFFFFF000;
                    break;
                default:
                    break;
            }
            break;
    }
}

void __fastcall__ rvExecute(struct RiscV* cpu) {
    switch (cpu->instr.opcode) {
        case 0x33: // R-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // add/sub
                    if (cpu->instr.funct7 == 0x00) { // add
                        PUT("add");
                        
                        cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v + cpu->regs[cpu->instr.rs2].v;
                    } else if (cpu->instr.funct7 == 0x20) { // sub
                        PUT("sub");

                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                        subU32fromU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    }
                    break;
                }
                case 0x4: { // xor
                    PUT("xor");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    xorU32withU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x6: { // or
                    PUT("or");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    orU32withU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x7: { // and
                    PUT("and");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    andU32withU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x1: { // sll
                    PUT("sll");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    sllU32withU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x5: { // srl/sra
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    
                    if (cpu->instr.funct7 == 0x00) { // srl
                        PUT("srl");
                        srlU32withU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    } else if (cpu->instr.funct7 == 0x20) { // sra
                        PUT("sra");
                        sraU32withU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    }
                   
                    break;
                }
                case 0x2: { // slt
                    PUT("slt");
                    
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    sltU32withU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    break;
                case 0x3: // sltu
                    PUT("sltu");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    sltuU32withU32(&cpu->regs[cpu->instr.rd], &cpu->regs[cpu->instr.rs2]);
                    break;
                }
            }

            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v);
            PUT(",");
            PUTR(cpu->instr.rs1, cpu->regs[cpu->instr.rs1].v);
            PUT(",");
            PUTR(cpu->instr.rs2, cpu->regs[cpu->instr.rs2].v);
            NEXT_LINE();
            break;
        case 0x13: // I-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // addi
                    PUT("addi");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm.v;
                    break;
                }
                case 0x4: { // xori
                    PUT("xori");
                    
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    xorImm16withU32(&cpu->regs[cpu->instr.rd], cpu->instr.imm.b);
                    break;
                }
                case 0x6: { // ori
                    PUT("ori");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    orImm16withU32(&cpu->regs[cpu->instr.rd], cpu->instr.imm.b);
                    break;
                }
                case 0x7: {// andi
                    PUT("andi");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    andImm16withU32(&cpu->regs[cpu->instr.rd], cpu->instr.imm.b);
                    break;
                }
                case 0x1: { // slli
                    PUT("slli");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    sllImm16withU32(&cpu->regs[cpu->instr.rd], cpu->instr.imm.b);
                    break;
                }
                case 0x5: { // srli/srai
                    if (cpu->instr.funct7 == 0x00) { // srli
                        PUT("srli");

                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                        srlImm16withU32(&cpu->regs[cpu->instr.rd], cpu->instr.imm.b);
                    } else if (cpu->instr.funct7 == 0x20) { // srai
                        PUT("srai");

                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                        sraImm16withU32(&cpu->regs[cpu->instr.rd], cpu->instr.imm.b);
                    }
                    break;
                }
                case 0x2: { // slti
                    PUT("slti");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    sltImm16withU32(&cpu->regs[cpu->instr.rd], cpu->instr.imm.b);
                    break;
                }
                case 0x3: { // sltiu
                    PUT("sltiu");

                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1];
                    sltuImm16withU32(&cpu->regs[cpu->instr.rd], cpu->instr.imm.b);
                    break;
                default:
                    break;
            }
            
            PUTR(cpu->instr.rd, &cpu->regs[cpu->instr.rd]);
            PUTR(cpu->instr.rs1, &cpu->regs[cpu->instr.rs1]);
            PUTI(&cpu->instr.imm);
            PUT((unsigned char*)' ');
            break;
        case 0x03: // Load instructions
            
            break;
        
    }
}

static void __fastcall__ subU32fromU32(u32* dst, const u32* src) {
    borrow = 0;

    for (i = 0; i < 4; ++i) {
        // Promote to signed int so we can detect borrow
        int diff = (int)dst->b[i] - (int)src->b[i] - borrow;
        if (diff < 0) {
            diff += 256;    // wrap underflow
            borrow = 1;
        } else {
            borrow = 0;
        }
        dst->b[i] = (unsigned char)diff;
    }
}

static void __fastcall__ xorU32withU32(u32* dst, const u32* src) {
    for (i = 0; i < 4; ++i) {
        dst->b[i] ^= src->b[i];
    }
}

static void __fastcall__ orU32withU32(u32* dst, const u32* src) {
    for (i = 0; i < 4; ++i) {
        dst->b[i] |= src->b[i];
    }
}

static void __fastcall__ andU32withU32(u32* dst, const u32* src) {
    for (i = 0; i < 4; ++i) {
        dst->b[i] &= src->b[i];
    }
}

static void __fastcall__ sllU32withU32(u32* dst, const u32* src) {
    shift = src->b[0] & 31;

    if (shift >= 32) {
        // Shift of 32 or more results in zero
        for (i = 0; i++ < 4;)
            dst->b[i] = 0;
        return;
    }

    while (shift >= 8) {
        // Shift full bytes
        dst->b[3] = dst->b[2];
        dst->b[2] = dst->b[1];
        dst->b[1] = dst->b[0];
        dst->b[0] = 0;
        shift -= 8;
    }

    if (shift > 0) {
        // Shift remaining bits
        carry = 0;
        
        for (i = 0; i++ < 4;) {
            newCarry = dst->b[i] >> (8 - shift);
            dst->b[i] = (dst->b[i] << shift) | carry;
            carry = newCarry;
        }
    }
}

static void __fastcall__ srlU32withU32(u32* dst, const u32* src) {
    shift = src->b[0] & 31;
   
    if (shift >= 32) {
        for (i = 0; i++ < 4;) 
            dst->b[i] = 0;
        return;
    }

    while (shift >= 8) {
        dst->b[0] = dst->b[1];
        dst->b[1] = dst->b[2];
        dst->b[2] = dst->b[3];
        dst->b[3] = 0;
        shift -= 8;
    }

    if (shift > 0) {
        carry = 0;
        
        for (i = 4; i-- > 0;) {
            newCarry = dst->b[i] << (8 - shift);
            dst->b[i] = (dst->b[i] >> shift) | carry;
            carry = newCarry;
        }
    }
}

static void __fastcall__ sraU32withU32(u32* dst, const u32* src) {
    unsigned char sign = dst->b[3] & 0x80; // top bit before shift
    shift = src->b[0] & 31;
    
    if (shift >= 32) {
        // Fill with all 1s if negative, else all 0s
        unsigned char fill = sign ? 0xFF : 0x00;
        
        for (i = 0; i++ < 4;) 
            dst->b[i] = fill;
        return;
    }

    while (shift >= 8) {
        dst->b[0] = dst->b[1];
        dst->b[1] = dst->b[2];
        dst->b[2] = dst->b[3];
        dst->b[3] = sign ? 0xFF : 0x00;
        shift -= 8;
    }

    if (shift > 0) {
        carry = sign ? 0xFF << (8 - shift) : 0x00;
        
        for (i = 4; i-- > 0;) {
            newCarry = dst->b[i] << (8 - shift);
            dst->b[i] = (dst->b[i] >> shift) | carry;
            carry = newCarry;
        }
    }
}

static void __fastcall__ sltU32withU32(u32* dst, const u32* src) {
    unsigned char signDst = dst->b[3] & 0x80;
    unsigned char signSrc = src->b[3] & 0x80;

    if (signDst != signSrc) {
        dst->b[0] = signDst ? 1 : 0;
        dst->b[1] = 0;
        dst->b[2] = 0;
        dst->b[3] = 0;
        return;
    }

    sltuU32withU32(dst, src);
}

static void __fastcall__ sltuU32withU32(u32* dst, const u32* src) {
    borrow = (dst->b[0] < src->b[0]);
    borrow = (dst->b[1] < (unsigned char)(src->b[1] + borrow));
    borrow = (dst->b[2] < (unsigned char)(src->b[2] + borrow));
    borrow = (dst->b[3] < (unsigned char)(src->b[3] + borrow));

    dst->b[0] = borrow;
    dst->b[1] = 0;
    dst->b[2] = 0;
    dst->b[3] = 0;
}

static void __fastcall__ xorImm16withU32(u32* dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    makeImm4fromImm2(imm_bytes);

    xorU32withU32(dst, &imm4);
}

static void __fastcall__ orImm16withU32(u32* dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    makeImm4fromImm2(imm_bytes);

    orU32withU32(dst, &imm4);
}

static void __fastcall__ andImm16withU32(u32* dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    makeImm4fromImm2(imm_bytes);

    andU32withU32(dst, &imm4);
}

static void __fastcall__ sllImm16withU32(u32* dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    makeImm4fromImm2(imm_bytes);

    sllU32withU32(dst, &imm4);
}

static void __fastcall__ srlImm16withU32(u32* dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    makeImm4fromImm2(imm_bytes);

    srlU32withU32(dst, &imm4);
}

static void __fastcall__ sraImm16withU32(u32* dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    makeImm4fromImm2(imm_bytes);

    sraU32withU32(dst, &imm4);
}

static void __fastcall__ sltImm16withU32(u32* dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    makeImm4fromImm2(imm_bytes);

    sltU32withU32(dst, &imm4);
}

static void __fastcall__ sltuImm16withU32(u32* dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    makeImm4fromImm2(imm_bytes);

    sltuU32withU32(dst, &imm4);
}
