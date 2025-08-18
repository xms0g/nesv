#include "riscv.h"
#include "nesio.h"
#include "../libs/neslib.h"

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
    cpu->regs[R_SP].v = 0x80000400;
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
        case 0x67: // jalr
        case 0x3: { // Load Instructions
            switch (cpu->instr.funct3) {
                case 0x0: // addi/lb/jalr
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
        case 0x63: { // B-type
            switch (cpu->instr.funct3) {
                case 0x0: // beq
                case 0x1: // bne
                case 0x4: // blt
                case 0x5: // bge
                case 0x6: // bltu
                case 0x7: { // bgeu
                    long imm = 0;

                    imm |= ((raw->v >> 31) & 0x1) << 12;  // imm[12]
                    imm |= ((raw->v >> 25) & 0x3F) << 5;  // imm[10:5]
                    imm |= ((raw->v >> 8)  & 0xF) << 1;   // imm[4:1]
                    imm |= ((raw->v >> 7)  & 0x1) << 11;  // imm[11]

                    // sign-extend 13-bit immediate
                    cpu->instr.imm = (imm << 19) >> 19;

                    cpu->instr.rs2 = (raw->v >> 20) & 0x1f;
                    break;
                }     
            }
            break;
        }
        case 0x6F: { // jal
            long imm = 0;

            imm |= ((raw->v >> 31) & 0x1) << 20;   // imm[20]
            imm |= ((raw->v >> 21) & 0x3FF) << 1;  // imm[10:1]
            imm |= ((raw->v >> 20) & 0x1) << 11;   // imm[11]
            imm |= ((raw->v >> 12) & 0xFF) << 12;  // imm[19:12]

            // sign-extend 21-bit immediate
            cpu->instr.imm = (imm << 11) >> 11;
            break;
        }
        case 0x37: // lui
        case 0x17: // auipc
            cpu->instr.imm = (raw->v >> 12) & 0xFFFFF;
            break;    
    }
}

void __fastcall__ rvExecute(struct RiscV* cpu, unsigned char* hasJump) {
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
                    PUT("lh");

                    addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd].v = (long)(signed int)(
                        busLoad(&cpu->bus, addr.v, 16)->b[0] | 
                        (busLoad(&cpu->bus, addr.v, 16)->b[1] << 8));
                    break;
                }
                case 0x2: // lw
                    PUT("lw");

                    addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd].v = busLoad(&cpu->bus, addr.v, 32)->v;
                    break;
                case 0x4: // lbu
                    PUT("lbu");

                    addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd].b[0] = busLoad(&cpu->bus, addr.v, 8)->b[0];
                    break;
                case 0x5: // lhu
                    PUT("lhu");

                    addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    cpu->regs[cpu->instr.rd].v = busLoad(&cpu->bus, addr.v, 16)->b[0] | (busLoad(&cpu->bus, addr.v, 16)->b[1] << 8);
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
                    break;
                }
                case 0x1: { // sh
                    PUT("sh");

                    addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    busStore(&cpu->bus, addr.v, 16, &cpu->regs[cpu->instr.rs2]);
                    break;
                }
                case 0x2: { // sw
                    PUT("sw");

                    addr.v = cpu->regs[cpu->instr.rs1].v + cpu->instr.imm;
                    busStore(&cpu->bus, addr.v, 32, &cpu->regs[cpu->instr.rs2]);
                    break;
                }
            }

            PUTR(cpu->instr.rs1, cpu->regs[cpu->instr.rs1].v); NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            PUTR(cpu->instr.rs2, cpu->regs[cpu->instr.rs2].v);NEXT_LINE();
            break;
        }
        case 0x63: { // B-type Instructions
            switch (cpu->instr.funct3) {
                case 0x0: { // beq
                    if (cpu->regs[cpu->instr.rs1].v == cpu->regs[cpu->instr.rs2].v) {
                        cpu->pc += cpu->instr.imm;
                        *hasJump = 1;
                    }
                    break;
                }
                case 0x1: { // bne
                    if (cpu->regs[cpu->instr.rs1].v != cpu->regs[cpu->instr.rs2].v) {
                        cpu->pc += cpu->instr.imm;
                        *hasJump = 1;
                    }
                    break;
                }
                case 0x4: { // blt
                    if ((signed char)cpu->regs[cpu->instr.rs1].v < (signed char)cpu->regs[cpu->instr.rs2].v) {
                        cpu->pc += cpu->instr.imm;
                        *hasJump = 1;
                    }
                    break;
                }
                case 0x5: { //bge
                    if ((signed char)cpu->regs[cpu->instr.rs1].v >= (signed char)cpu->regs[cpu->instr.rs2].v) {
                        cpu->pc += cpu->instr.imm;
                        *hasJump = 1;
                    }
                    break;
                }
                case 0x6: { // bltu
                    if (cpu->regs[cpu->instr.rs1].v < cpu->regs[cpu->instr.rs2].v) {
                        cpu->pc += cpu->instr.imm;
                        *hasJump = 1;
                    }
                    break;
                }
                case 0x7: { //bgeu
                    if (cpu->regs[cpu->instr.rs1].v >= cpu->regs[cpu->instr.rs2].v) {
                        cpu->pc += cpu->instr.imm;
                        *hasJump = 1;
                    }
                    break;
                }
            }
            PUTR(cpu->instr.rs1, cpu->regs[cpu->instr.rs1].v);NEXT_LINE();
            PUTR(cpu->instr.rs2, cpu->regs[cpu->instr.rs2].v);NEXT_LINE();
            PUTUI(cpu->pc);NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            break;
        }
        case 0x6F: { // jal
            PUT("jal");

            cpu->regs[cpu->instr.rd].v = cpu->pc + 4;
            cpu->pc = cpu->pc + cpu->instr.imm;
            *hasJump = 1;

            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v);NEXT_LINE();
            PUTUI(cpu->pc);NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            break;
        }
        case 0x67: { // jalr
            PUT("jalr");

            cpu->regs[cpu->instr.rd].v = cpu->pc + 4;
            cpu->pc = (cpu->regs[cpu->instr.rs1].v + cpu->instr.imm) & ~1u;
            *hasJump = 1;

            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v);NEXT_LINE();
            PUTR(cpu->instr.rs1, cpu->regs[cpu->instr.rs1].v);NEXT_LINE();
            PUTUI(cpu->pc);NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            break;
        }
        case 0x37: { // lui
            PUT("lui");

            cpu->regs[cpu->instr.rd].v = cpu->instr.imm << 12;
            
            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v);NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            break;
        }
        case 0x17: { // auipc
            PUT("auipc");

            cpu->regs[cpu->instr.rd].v = cpu->pc + cpu->instr.imm << 12;
            
            PUTR(cpu->instr.rd, cpu->regs[cpu->instr.rd].v);NEXT_LINE();
            PUTUI(cpu->pc);NEXT_LINE();
            PUTSI(cpu->instr.imm);NEXT_LINE();
            break; 
        }   
    }
}