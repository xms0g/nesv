#include "riscv.h"
#include <string.h>
#include "nesio.h"
#include "../libs/neslib.h"

#define OPC(raw) ((raw) & 0x7F)
#define RD(raw) (((raw) >> 7) & 0x1F)
#define FUNCT3(raw) (((raw) >> 12) & 0x07)
#define RS1(raw) (((raw) >> 15) & 0x1F)
#define RS2(raw) (((raw) >> 20) & 0x1F)
#define FUNCT7(raw) (((raw) >> 25) & 0x7F)
#define IMM_I(raw) ((((raw) >> 20) & 0xFFF))
#define IMM_S(raw) (((((raw) >> 25) & 0x7F) << 5) | ((((raw) >> 7) & 0x1F)))
#define IMM_U(raw) (((raw) >> 12) & 0xFFFFF)
#define SIGN_EXT_12(imm) ((long)((imm) << 20) >> 20)
#define SIGN_EXT_13(imm) ((long)((imm) << 19) >> 19)
#define SIGN_EXT_21(imm) ((long)((imm) << 11) >> 11)

#pragma bss-name(push, "ZEROPAGE")
unsigned char x;
unsigned char y;
unsigned char hasJump;
unsigned long addr;
unsigned long* instr;
#pragma bss-name(pop)

static void __fastcall__ debug_put(const char* str) {
#ifdef DEBUG
    PUT(str);
#endif
}

void __fastcall__ rvInit(struct RiscV* cpu) {
    SETXY(1, 1);
    
    hasJump = 0;
    
    memset(cpu->regs, 0, sizeof(unsigned long) * 32);
    
    cpu->regs[X0] = 0x0;
    // Set R_SP = 0x80000400
    cpu->regs[R_SP] = DRAM_BASE + DRAM_SIZE;
    cpu->pc = DRAM_BASE;
}

void __fastcall__ rvRun(struct RiscV* cpu) {
    while (1) { 
        instr = rvFetch(cpu);

		if (*instr == 0) break;

        rvDecode(cpu, instr);

        rvExecute(cpu);

		if (!hasJump)
			cpu->pc += 4;
		
		hasJump = 0;   
    }
}

unsigned long* __fastcall__ rvFetch(const struct RiscV* cpu) {
    return busLoad(&cpu->bus, &cpu->pc, 32);
}

void __fastcall__ rvDecode(struct RiscV* cpu, const unsigned long* raw) {
    cpu->instr.opcode = OPC(*raw);
    cpu->instr.rd = RD(*raw);
    cpu->instr.funct3 = FUNCT3(*raw);
    cpu->instr.rs1 = RS1(*raw);
   
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
                    cpu->instr.rs2 = RS2(*raw);
                    cpu->instr.funct7 = FUNCT7(*raw);
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
                    cpu->instr.imm = IMM_I(*raw);
                    cpu->instr.imm = SIGN_EXT_12(cpu->instr.imm);
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
                    cpu->instr.rs2 = RS2(*raw);
                    cpu->instr.imm = IMM_S(*raw);
                    cpu->instr.imm = SIGN_EXT_12(cpu->instr.imm);
                    break;
                }
            }
            break;
        }
        case 0x63: { // B-type
            hasJump = 1;
            
            switch (cpu->instr.funct3) {
                case 0x0: // beq
                case 0x1: // bne
                case 0x4: // blt
                case 0x5: // bge
                case 0x6: // bltu
                case 0x7: { // bgeu
                    long imm = 0;
                    cpu->instr.rs2 = RS2(*raw);
                    
                    imm |= ((*raw >> 31) & 0x1) << 12;  // imm[12]
                    imm |= ((*raw >> 25) & 0x3F) << 5;  // imm[10:5]
                    imm |= ((*raw >> 8)  & 0xF) << 1;   // imm[4:1]
                    imm |= ((*raw >> 7)  & 0x1) << 11;  // imm[11]

                    // sign-extend 13-bit immediate
                    cpu->instr.imm = SIGN_EXT_13(imm);
                    break;
                }     
            }
            break;
        }
        case 0x6F: { // jal
            long imm = 0;
            hasJump = 1;

            imm |= ((*raw >> 31) & 0x1) << 20;   // imm[20]
            imm |= ((*raw >> 21) & 0x3FF) << 1;  // imm[10:1]
            imm |= ((*raw >> 20) & 0x1) << 11;   // imm[11]
            imm |= ((*raw >> 12) & 0xFF) << 12;  // imm[19:12]

            // sign-extend 21-bit immediate
            cpu->instr.imm = SIGN_EXT_21(imm);
            break;
        }
        case 0x67: { // jalr
            hasJump = 1;

            switch (cpu->instr.funct3) {
                case 0x0: { 
                    cpu->instr.imm = IMM_I(*raw);
                    cpu->instr.imm = SIGN_EXT_12(cpu->instr.imm);
                    break;
                }
            }
            break;
        }
        case 0x37: // lui
        case 0x17: // auipc
            cpu->instr.imm = IMM_U(*raw);
            break;    
    }
}

void __fastcall__ rvExecute(struct RiscV* cpu) {
#ifdef DEBUG
    SETXY(1, y);
#endif
    switch (cpu->instr.opcode) {
        case 0x33: { // R-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // add/sub
                    if (cpu->instr.funct7 == 0x00) { // add
                        debug_put("add");
                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] + cpu->regs[cpu->instr.rs2];
                    } else if (cpu->instr.funct7 == 0x20) { // sub
                        debug_put("sub");
                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] - cpu->regs[cpu->instr.rs2];
                    }
                    break;
                }
                case 0x4: { // xor
                    debug_put("xor");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] ^ cpu->regs[cpu->instr.rs2];
                    break;
                }
                case 0x6: { // or
                    debug_put("or");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] | cpu->regs[cpu->instr.rs2];
                    break;
                }
                case 0x7: { // and
                    debug_put("and");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] & cpu->regs[cpu->instr.rs2];
                    break;
                }
                case 0x1: { // sll
                    debug_put("sll");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] << (cpu->regs[cpu->instr.rs2] & 0x1F);
                    break;
                }
                case 0x5: { // srl/sra
                    if (cpu->instr.funct7 == 0x00) { // srl
                        debug_put("srl");
                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] >> (cpu->regs[cpu->instr.rs2] & 0x1F);
                    } else if (cpu->instr.funct7 == 0x20) { // sra
                        debug_put("sra");
                        cpu->regs[cpu->instr.rd] = (long)cpu->regs[cpu->instr.rs1] >> (cpu->regs[cpu->instr.rs2] & 0x1F);
                    }
                    break;
                }
                case 0x2: { // slt
                    debug_put("slt");
                    cpu->regs[cpu->instr.rd] = (long)cpu->regs[cpu->instr.rs1] < (long)cpu->regs[cpu->instr.rs2] ? 1: 0;
                    break;
                }
                case 0x3: { // sltu
                    debug_put("sltu");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] < cpu->regs[cpu->instr.rs2] ? 1: 0;
                    break;
                }
            }
            #ifdef DEBUG
            PUTR(cpu->instr.rd);
            PUTR(cpu->instr.rs1);
            PUTR(cpu->instr.rs2);
            NEXT_LINE(x);
            #endif
            break;
        }
        case 0x13: { // I-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // addi
                    debug_put("addi");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    break;
                }
                case 0x4: { // xori
                    debug_put("xori");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] ^ cpu->instr.imm;
                    break;
                }
                case 0x6: { // ori
                    debug_put("ori");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] | cpu->instr.imm;
                    break;
                }
                case 0x7: {// andi
                    debug_put("andi");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] & cpu->instr.imm;
                    break;
                }
                case 0x1: { // slli
                    debug_put("slli");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] << (cpu->instr.imm & 0x1F);
                    break;
                }
                case 0x5: { // srli/srai
                    if (cpu->instr.funct7 == 0x00) { // srli
                        debug_put("srli");
                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] >> (cpu->instr.imm & 0x1F);
                    } else if (cpu->instr.funct7 == 0x20) { // srai
                        debug_put("srai");
                        cpu->regs[cpu->instr.rd] = (long)cpu->regs[cpu->instr.rs1] >> (cpu->instr.imm & 0x1F);
                    }
                    break;
                }
                case 0x2: { // slti
                    debug_put("slti");
                    cpu->regs[cpu->instr.rd] = (long)cpu->regs[cpu->instr.rs1] < (long)cpu->instr.imm ? 1: 0;
                    break;
                }
                case 0x3: { // sltiu
                    debug_put("sltiu");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] < cpu->instr.imm ? 1: 0;
                    break;
                }
            }
            #ifdef DEBUG
            PUTR(cpu->instr.rd);
            PUTR(cpu->instr.rs1);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            #endif
            break;
        }
        case 0x03: { // Load Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // lb
                    debug_put("lb");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = (long)(signed char)*busLoad(&cpu->bus, &addr, 8);
                    break;
                }
                case 0x1: { // lh
                    debug_put("lh");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = (long)(int)*busLoad(&cpu->bus, &addr, 16);
                    break;
                }
                case 0x2: { // lw
                    debug_put("lw");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = (long)*busLoad(&cpu->bus, &addr, 32);
                    break;
                }   
                case 0x4: { // lbu
                    debug_put("lbu");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = *busLoad(&cpu->bus, &addr, 8);
                    break;
                } 
                case 0x5: { // lhu
                    debug_put("lhu");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = *busLoad(&cpu->bus, &addr, 16);
                    break;
                }
            }
            #ifdef DEBUG
            PUTR(cpu->instr.rd);
            PUTLS(cpu->instr.imm, cpu->instr.rs1);
            NEXT_LINE(x);
            #endif
            break;
        }
        case 0x23: { // Store Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // sb
                    debug_put("sb");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    busStore(&cpu->bus, &addr, 8, &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x1: { // sh
                    debug_put("sh");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    busStore(&cpu->bus, &addr, 16, &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x2: { // sw
                    debug_put("sw");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    busStore(&cpu->bus, &addr, 32, &cpu->regs[cpu->instr.rs2]);
                    break;
                }
            }
            #ifdef DEBUG
            PUTR(cpu->instr.rs2);
            PUTLS(cpu->instr.imm, cpu->instr.rs1);
            NEXT_LINE(x);
            #endif
            break;
        }
        case 0x63: { // B-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // beq
                    debug_put("beq");
                    if (cpu->regs[cpu->instr.rs1] == cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x1: { // bne
                    debug_put("bne");
                    if (cpu->regs[cpu->instr.rs1] != cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x4: { // blt
                    debug_put("blt");
                    if ((long)cpu->regs[cpu->instr.rs1] < (long)cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x5: { //bge
                    debug_put("bge");
                    if ((long)cpu->regs[cpu->instr.rs1] >= (long)cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x6: { // bltu
                    debug_put("bltu");
                    if (cpu->regs[cpu->instr.rs1] < cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x7: { //bgeu
                    debug_put("bgeu");
                    if (cpu->regs[cpu->instr.rs1] >= cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
            }
            #ifdef DEBUG
            PUTR(cpu->instr.rs1);
            PUTR(cpu->instr.rs2);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            #endif
            break;
        }
        case 0x6F: { // jal
            cpu->regs[cpu->instr.rd] = cpu->pc + 4;
            cpu->pc = cpu->pc + cpu->instr.imm;
            
            #ifdef DEBUG
            PUT("jal");
            PUTR(cpu->instr.rd);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            #endif
            break;
        }
        case 0x67: { // jalr
            cpu->regs[cpu->instr.rd] = cpu->pc + 4;
            cpu->pc = (cpu->regs[cpu->instr.rs1] + cpu->instr.imm) & ~1u;

            #ifdef DEBUG
            PUT("jalr");
            PUTR(cpu->instr.rd);
            PUTR(cpu->instr.rs1);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            #endif
            break;
        }
        case 0x37: { // lui
            cpu->regs[cpu->instr.rd] = cpu->instr.imm << 12;
            
            #ifdef DEBUG
            PUT("lui");
            PUTR(cpu->instr.rd);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            #endif
            break;
        }
        case 0x17: { // auipc
            cpu->regs[cpu->instr.rd] = cpu->pc + cpu->instr.imm << 12;
            
            #ifdef DEBUG
            PUT("auipc");
            PUTR(cpu->instr.rd);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            #endif
            break; 
        }   
    }
}

void __fastcall__ rvDumpReg(const struct RiscV* cpu) {
    static unsigned char i = 0;
    unsigned char currenty = y;

#ifdef DEBUG
    SETXY(1, currenty + 2);
#endif

    for (; i < 32; ++i) {
        if (y > 27) {
#ifdef DEBUG
            y = currenty + 2;
#else
            y = currenty;
#endif
            NEXT_CHAR(15);
        }
        
        print("x");
        uprint(i);
        print(":");
        if (i == 2) {
            print("0x");
            xprint(cpu->regs[i]);
        } else {
            sprint(cpu->regs[i]);
        }
        NEXT_LINE(x);
    }

    NEXT_LINE(x);
    print("PC: ");print("0x");xprint(cpu->pc);
}