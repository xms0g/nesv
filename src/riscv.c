#include "riscv.h"
#include <string.h>
#include "nesio.h"
#include "../libs/neslib.h"

#pragma bss-name(push, "ZEROPAGE")
unsigned char x;
unsigned char y;
unsigned char hasJump;
unsigned long addr;
unsigned long* instr;
#pragma bss-name(pop)

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
    cpu->instr.opcode = *raw & 0x7F;
    cpu->instr.rd = (*raw >> 7) & 0x1f;
    cpu->instr.funct3 = (*raw >> 12) & 0x07;
    cpu->instr.rs1 = (*raw >> 15) & 0x1f;
   
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
                    cpu->instr.rs2 = (*raw >> 20) & 0x1f;
                    cpu->instr.funct7 = (*raw >> 25) & 0x7F;
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
                    cpu->instr.imm = (*raw >> 20) & 0xFFF;
                    
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
                    cpu->instr.rs2 = (*raw >> 20) & 0x1f;
                    cpu->instr.imm = (((*raw >> 25) & 0x7F) << 5) | ((*raw >> 7) & 0x1f);

                    if (cpu->instr.imm & 0x800)
                        cpu->instr.imm |= 0xFFFFF000;
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

                    imm |= ((*raw >> 31) & 0x1) << 12;  // imm[12]
                    imm |= ((*raw >> 25) & 0x3F) << 5;  // imm[10:5]
                    imm |= ((*raw >> 8)  & 0xF) << 1;   // imm[4:1]
                    imm |= ((*raw >> 7)  & 0x1) << 11;  // imm[11]

                    // sign-extend 13-bit immediate
                    cpu->instr.imm = (imm << 19) >> 19;

                    cpu->instr.rs2 = (*raw >> 20) & 0x1f;
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
            cpu->instr.imm = (imm << 11) >> 11;
            break;
        }
        case 0x67: { // jalr
            hasJump = 1;

            switch (cpu->instr.funct3) {
                case 0x0: { 
                    cpu->instr.imm = (*raw >> 20) & 0xFFF;
                    
                    if (cpu->instr.imm & 0x800)
                        cpu->instr.imm |= 0xFFFFF000;
                    break;
                }
            }
            break;
        }
        case 0x37: // lui
        case 0x17: // auipc
            cpu->instr.imm = (*raw >> 12) & 0xFFFFF;
            break;    
    }
}

void __fastcall__ rvExecute(struct RiscV* cpu) {
    SETXY(1, y);

    switch (cpu->instr.opcode) {
        case 0x33: { // R-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // add/sub
                    if (cpu->instr.funct7 == 0x00) { // add
                        PUT("add");
                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] + cpu->regs[cpu->instr.rs2];
                    } else if (cpu->instr.funct7 == 0x20) { // sub
                        PUT("sub");
                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] - cpu->regs[cpu->instr.rs2];
                    }
                    break;
                }
                case 0x4: { // xor
                    PUT("xor");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] ^ cpu->regs[cpu->instr.rs2];
                    break;
                }
                case 0x6: { // or
                    PUT("or");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] | cpu->regs[cpu->instr.rs2];
                    break;
                }
                case 0x7: { // and
                    PUT("and");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] & cpu->regs[cpu->instr.rs2];
                    break;
                }
                case 0x1: { // sll
                    PUT("sll");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] << (cpu->regs[cpu->instr.rs2] & 0x1F);
                    break;
                }
                case 0x5: { // srl/sra
                    if (cpu->instr.funct7 == 0x00) { // srl
                        PUT("srl");
                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] >> (cpu->regs[cpu->instr.rs2] & 0x1F);
                    } else if (cpu->instr.funct7 == 0x20) { // sra
                        PUT("sra");
                        cpu->regs[cpu->instr.rd] = (long)cpu->regs[cpu->instr.rs1] >> (cpu->regs[cpu->instr.rs2] & 0x1F);
                    }
                    break;
                }
                case 0x2: { // slt
                    PUT("slt");
                    cpu->regs[cpu->instr.rd] = (long)cpu->regs[cpu->instr.rs1] < (long)cpu->regs[cpu->instr.rs2] ? 1: 0;
                    break;
                }
                case 0x3: { // sltu
                    PUT("sltu");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] < cpu->regs[cpu->instr.rs2] ? 1: 0;
                    break;
                }
            }
            PUTR(cpu->instr.rd);
            PUTR(cpu->instr.rs1);
            PUTR(cpu->instr.rs2);
            NEXT_LINE(x);
            break;
        }
        case 0x13: { // I-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // addi
                    PUT("addi");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    break;
                }
                case 0x4: { // xori
                    PUT("xori");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] ^ cpu->instr.imm;
                    break;
                }
                case 0x6: { // ori
                    PUT("ori");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] | cpu->instr.imm;
                    break;
                }
                case 0x7: {// andi
                    PUT("andi");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] & cpu->instr.imm;
                    break;
                }
                case 0x1: { // slli
                    PUT("slli");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] << (cpu->instr.imm & 0x1F);
                    break;
                }
                case 0x5: { // srli/srai
                    if (cpu->instr.funct7 == 0x00) { // srli
                        PUT("srli");
                        cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] >> (cpu->instr.imm & 0x1F);
                    } else if (cpu->instr.funct7 == 0x20) { // srai
                        PUT("srai");
                        cpu->regs[cpu->instr.rd] = (long)cpu->regs[cpu->instr.rs1] >> (cpu->instr.imm & 0x1F);
                    }
                    break;
                }
                case 0x2: { // slti
                    PUT("slti");
                    cpu->regs[cpu->instr.rd] = (long)cpu->regs[cpu->instr.rs1] < (long)cpu->instr.imm ? 1: 0;
                    break;
                }
                case 0x3: { // sltiu
                    PUT("sltiu");
                    cpu->regs[cpu->instr.rd] = cpu->regs[cpu->instr.rs1] < cpu->instr.imm ? 1: 0;
                    break;
                }
            }
            PUTR(cpu->instr.rd);
            PUTR(cpu->instr.rs1);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            break;
        }
        case 0x03: { // Load Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // lb
                    PUT("lb");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = (long)(signed char)*busLoad(&cpu->bus, &addr, 8);
                    break;
                }
                case 0x1: { // lh
                    PUT("lh");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = (long)(int)*busLoad(&cpu->bus, &addr, 16);
                    break;
                }
                case 0x2: { // lw
                    PUT("lw");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = (long)*busLoad(&cpu->bus, &addr, 32);
                    break;
                }   
                case 0x4: { // lbu
                    PUT("lbu");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = *busLoad(&cpu->bus, &addr, 8);
                    break;
                } 
                case 0x5: { // lhu
                    PUT("lhu");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd] = *busLoad(&cpu->bus, &addr, 16);
                    break;
                }
            }
            PUTR(cpu->instr.rd);
            PUTLS(cpu->instr.imm, cpu->instr.rs1);
            NEXT_LINE(x);
            break;
        }
        case 0x23: { // Store Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // sb
                    PUT("sb");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    busStore(&cpu->bus, &addr, 8, &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x1: { // sh
                    PUT("sh");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    busStore(&cpu->bus, &addr, 16, &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x2: { // sw
                    PUT("sw");
                    addr = cpu->regs[cpu->instr.rs1] + cpu->instr.imm;
                    busStore(&cpu->bus, &addr, 32, &cpu->regs[cpu->instr.rs2]);
                    break;
                }
            }
            PUTR(cpu->instr.rs2);
            PUTLS(cpu->instr.imm, cpu->instr.rs1);
            NEXT_LINE(x);
            break;
        }
        case 0x63: { // B-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // beq
                    PUT("beq");
                    if (cpu->regs[cpu->instr.rs1] == cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x1: { // bne
                    PUT("bne");
                    if (cpu->regs[cpu->instr.rs1] != cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x4: { // blt
                    PUT("blt");
                    if ((long)cpu->regs[cpu->instr.rs1] < (long)cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x5: { //bge
                    PUT("bge");
                    if ((long)cpu->regs[cpu->instr.rs1] >= (long)cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x6: { // bltu
                    PUT("bltu");
                    if (cpu->regs[cpu->instr.rs1] < cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
                case 0x7: { //bgeu
                    PUT("bgeu");
                    if (cpu->regs[cpu->instr.rs1] >= cpu->regs[cpu->instr.rs2]) {
                        cpu->pc += cpu->instr.imm;
                    }
                    break;
                }
            }
            PUTR(cpu->instr.rs1);
            PUTR(cpu->instr.rs2);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            break;
        }
        case 0x6F: { // jal
            PUT("jal");
            cpu->regs[cpu->instr.rd] = cpu->pc + 4;
            cpu->pc = cpu->pc + cpu->instr.imm;
            
            PUTR(cpu->instr.rd);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            break;
        }
        case 0x67: { // jalr
            PUT("jalr");
            cpu->regs[cpu->instr.rd] = cpu->pc + 4;
            cpu->pc = (cpu->regs[cpu->instr.rs1] + cpu->instr.imm) & ~1u;

            PUTR(cpu->instr.rd);
            PUTR(cpu->instr.rs1);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            break;
        }
        case 0x37: { // lui
            PUT("lui");
            cpu->regs[cpu->instr.rd] = cpu->instr.imm << 12;
            
            PUTR(cpu->instr.rd);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            
            break;
        }
        case 0x17: { // auipc
            PUT("auipc");
            cpu->regs[cpu->instr.rd] = cpu->pc + cpu->instr.imm << 12;
            
            PUTR(cpu->instr.rd);
            PUTSI(cpu->instr.imm);
            NEXT_LINE(x);
            break; 
        }   
    }
}

void __fastcall__ rvDumpReg(const struct RiscV* cpu) {
    unsigned char i = 0, currY = y;
    SETXY(1, currY + 2);

    for (; i < 32; ++i) {
        if (y > 27) {
            y = currY + 2;
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