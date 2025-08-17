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

#pragma bss-name(push, "ZEROPAGE")
static unsigned char x;
static unsigned char y;
static u32 addr;
#pragma bss-name(pop)

void __fastcall__ rvInit(struct RiscV* cpu) {
    x = 1, y = 1;
    vram_adr(NTADR_A(x, y));
    
    memset(cpu->regs, 0, sizeof(u32) * 32);
    
    cpu->regs[X0].v = 0x0;
    // Set R_SP = 0x80000400
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
    cpu->instr.rd = (raw->v >> 7) & 0x1f;
    cpu->instr.funct3 = (raw->v >> 12) & 0x07;
    cpu->instr.rs1 = (raw->v >> 15) & 0x1f;
   
    switch (cpu->instr.opcode) {
        case 0x33: { // R-type Instructions
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
        }
        case 0x13: // I-type Instructions
        case 0x3: { // Load Instructions
            switch (cpu->instr.funct3) {
                case 0x0: // addi/lb
                case 0x4: // xori/lbu
                case 0x6: // ori
                case 0x7: // andi
                case 0x1: // slli/lh
                case 0x5: // srli/srai/lhu
                case 0x2: // slti/lw
                case 0x3: { // sltiu
                    cpu->instr.imm = (raw->v >> 20) & 0xFFF;
                   
                    if (cpu->instr.imm & 0x800)
                        cpu->instr.imm |= 0xFFFFF000;
                    break;
                }
            }
            break;
        }
        case 0x23: { // S-type
            switch (cpu->instr.funct3) {
                case 0x0: // sb
                case 0x1: // sh
                case 0x2: { // sw
                    cpu->instr.rs2 = (raw->v >> 20) & 0x1f;
                    cpu->instr.imm = (((raw->v >> 25) & 0x7F) << 5) | ((raw->v >> 7) & 0x1f);
                    
                    if (cpu->instr.imm & 0x800) 
                        cpu->instr.imm |= 0xFFFFF000;
                    break;
                }
            }
            break;
        }
        case 0x63: // B-type
            break;
        case 0x6F:// J-type
            break;
        case 0x67:
            break;
        case 0x37: // lui
            cpu->instr.imm = (long)(raw->v >> 12) & 0xFFFFF;
            break;
        case 0x17: // auipc
            break;
            
    }
}

void __fastcall__ rvExecute(struct RiscV* cpu) {
    switch (cpu->instr.opcode) {
        case 0x33: { // R-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // add/sub
                    if (cpu->instr.funct7 == 0x00) { // add
                        PUT("add");
                        
                        cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v + cpu->regs[cpu->instr.rs2].v;
                    } else if (cpu->instr.funct7 == 0x20) { // sub
                        PUT("sub");
                        
                        cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v - cpu->regs[cpu->instr.rs2].v;
                    }
                    break;
                }
                case 0x4: { // xor
                    PUT("xor");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v ^ cpu->regs[cpu->instr.rs2].v;
                    break;
                }
                case 0x6: { // or
                    PUT("or");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v | cpu->regs[cpu->instr.rs2].v;
                    break;
                }
                case 0x7: { // and
                    PUT("and");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v & cpu->regs[cpu->instr.rs2].v;
                    break;
                }
                case 0x1: { // sll
                    PUT("sll");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v << (cpu->regs[cpu->instr.rs2].v & 0x1F);
                    break;
                }
                case 0x5: { // srl/sra
                    if (cpu->instr.funct7 == 0x00) { // srl
                        PUT("srl");
                        
                        cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v >> (cpu->regs[cpu->instr.rs2].v & 0x1F);
                    } else if (cpu->instr.funct7 == 0x20) { // sra
                        PUT("sra");

                        cpu->regs[cpu->instr.rd].v = (long)cpu->regs[cpu->instr.rs1].v >> (cpu->regs[cpu->instr.rs2].v & 0x1F);
                    }
                    break;
                }
                case 0x2: { // slt
                    PUT("slt");
                    
                    cpu->regs[cpu->instr.rd].v = (long)cpu->regs[cpu->instr.rs1].v < (long)cpu->regs[cpu->instr.rs2].v ? 1: 0;
                    break;
                }
                case 0x3: { // sltu
                    PUT("sltu");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v < cpu->regs[cpu->instr.rs2].v ? 1: 0;
                    break;
                }
            }

            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v); PUT(" ");
            PUTR(cpu->instr.rs1, cpu->regs[cpu->instr.rs1].v); PUT(" ");
            PUTR(cpu->instr.rs2, cpu->regs[cpu->instr.rs2].v);
            NEXT_LINE();
            break;
        }
        case 0x13: { // I-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // addi
                    PUT("addi");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    break;
                }
                case 0x4: { // xori
                    PUT("xori");
                    
                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v ^ cpu->instr.imm;
                    break;
                }
                case 0x6: { // ori
                    PUT("ori");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v | cpu->instr.imm;
                    break;
                }
                case 0x7: {// andi
                    PUT("andi");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v & cpu->instr.imm;
                    break;
                }
                case 0x1: { // slli
                    PUT("slli");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v << (cpu->instr.imm & 0x1F);
                    break;
                }
                case 0x5: { // srli/srai
                    if (cpu->instr.funct7 == 0x00) { // srli
                        PUT("srli");

                        cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v >> (cpu->instr.imm & 0x1F);
                    } else if (cpu->instr.funct7 == 0x20) { // srai
                        PUT("srai");

                        cpu->regs[cpu->instr.rd].v = (long)cpu->regs[cpu->instr.rs1].v >> (cpu->instr.imm & 0x1F);
                    }
                    break;
                }
                case 0x2: { // slti
                    PUT("slti");

                    cpu->regs[cpu->instr.rd].v = (long)cpu->regs[cpu->instr.rs1].v < (long)cpu->instr.imm ? 1: 0;
                    break;
                }
                case 0x3: { // sltiu
                    PUT("sltiu");

                    cpu->regs[cpu->instr.rd].v = cpu->regs[cpu->instr.rs1].v < cpu->instr.imm ? 1: 0;
                    break;
                }
            }
            
            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v);NEXT_LINE();
            PUTR(cpu->instr.rs1, cpu->regs[cpu->instr.rs1].v);NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            break;
        }
        case 0x03: { // Load Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // lb
                    PUT("lb");

                    addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd].v = (long)(signed char)busLoad(&cpu->bus, addr.v, 8)->b[0];
                    break;
                }
                case 0x1: { // lh
                    // u32 addr;
                    // addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm.v;
                    // cpu->regs[cpu->instr.rd].v = (long int)(busLoad(&cpu->bus, addr.v, 16)[0] | (busLoad(&cpu->bus, addr.v, 16)[1] << 8));
                    // break;
                }
                case 0x2: // lw
                    // u32 addr;
                    // addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm.v;
                    // cpu->regs[cpu->instr.rd].v = (*busLoad(&cpu->bus, addr.v, 32)).v;
                    // break;
                case 0x4: // lbu
                    break;
                case 0x5: // lhu
                    break;
            }

            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v);NEXT_LINE();
            PUTR(cpu->instr.rs1, cpu->regs[cpu->instr.rs1].v);NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            break;
        }
        case 0x23: { // Store Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // sb
                    PUT("sb");

                    addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    busStore(&cpu->bus, addr.v, 8, &cpu->regs[cpu->instr.rs2]);
                }
            }

            PUTR(cpu->instr.rs1, cpu->regs[cpu->instr.rs1].v); NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            PUTR(cpu->instr.rs2, cpu->regs[cpu->instr.rs2].v);NEXT_LINE();
            break;
        }
        case 0x37: { // lui
            PUT("lui");

            cpu->regs[cpu->instr.rd].v = cpu->instr.imm << 12;
            
            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v);NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            break;
        }
        case 0x17: // auipc
            break;    
        
    }
}