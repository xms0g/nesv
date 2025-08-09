#include "riscv.h"
#include "neslib.h"

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

typedef union {
    unsigned char b[4];
} u32;

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


static struct RiscV rm;

static int __fastcall__ vFetch(void);
static void __fastcall__ vDecode(const unsigned char raw[4]);
static void __fastcall__ vExecute(void);

static void __fastcall__ addU32toU32(u32 *dst, const u32 *src);
static void __fastcall__ addImm16toU32(u32 *dst, const unsigned char imm_bytes[2]);

static void __fastcall__ printOP(const unsigned char* op, unsigned char* x, unsigned char* y);
static void __fastcall__ printREG(const unsigned char reg, unsigned char* x, unsigned char* y);

void __fastcall__ vInit(void) {
    
    rm.regs[X0].b[0] = 0x0;
    rm.regs[R_SP].b[0] = 0xFF; // Stack pointer initialized to the top of RAM
    rm.regs[R_SP].b[1] = 0xFF;
}

void __fastcall__ vRun(const unsigned char* raw, unsigned char size) {
    unsigned int i;
    for (i = 0; i < size - 1; i += 4) { 
        //vFetch();

        vDecode(raw + i);

        vExecute();
        
    }
   
}

static int __fastcall__ vFetch(void) {
    // Fetch logic here
   
    return 0; // Placeholder return value
}

static void __fastcall__ vDecode(const unsigned char raw[4]) {
    rm.instr.opcode = raw[0] & 0x7F;
    rm.instr.rd = ((raw[0] >> 7) & 0x01) | ((raw[1] & 0x0F) << 1);
    rm.instr.funct3 = (raw[1] >> 4) & 0x07;
    rm.instr.rs1 = ((raw[1] >> 7) & 1) | ((raw[2] & 0x0F) << 1); 
   
    switch (rm.instr.opcode) {
        case 0x33: // R-type instructions
            rm.instr.rs2 = ((raw[2] >> 4) & 0x0F) | ((raw[3] & 0x01) << 4);
            rm.instr.funct7 = (raw[3] >> 1) & 0x7F;
            break;
        case 0x13: // I-type instructions
        case 0x3: // Load instructions
            switch (rm.instr.funct3) {
                case 0x0: // addi
                case 0x4: // xori
                case 0x6: // ori
                case 0x7: // andi
                case 0x1: // slli
                case 0x5: // srli/srai
                case 0x2: // slti
                case 0x3: // sltiu
                    rm.instr.imm.b[0] = raw[2] >> 4; // immediate low byte
                    rm.instr.imm.b[1] = raw[3];      // immediate high byte    
                    break;
                default:
                    break;
            }
            break;
        }
    
}

static void __fastcall__ vExecute(void) {
    static unsigned char x = 5,y = 0;
    
    switch (rm.instr.opcode) {
        case 0x33:
            switch (rm.instr.funct3) {
                case 0x0: // add/sub
                    if (rm.instr.funct7 == 0x00) { // add
                        printOP("add", &x, &y);

                        addU32toU32(&rm.regs[rm.instr.rd], &rm.regs[rm.instr.rs1]);
                        addU32toU32(&rm.regs[rm.instr.rd], &rm.regs[rm.instr.rs2]);
                    } else if (rm.instr.funct7 == 0x20) { // sub
                        printOP("sub", &x, &y);
                    }
                    break;
                case 0x4: // xor
                    printOP("xor", &x, &y);
                    break;
                case 0x6: // or
                    printOP("or", &x, &y);
                    break;
                case 0x7: // and
                    printOP("and", &x, &y);
                    break;
                case 0x1: // sll
                    printOP("sll", &x, &y);
                    break;
                case 0x5: // srl/sra
                    if (rm.instr.funct7 == 0x00) { // srl
                        printOP("srl", &x, &y);
                    } else if (rm.instr.funct7 == 0x20) { // sra
                        printOP("sra", &x, &y);
                    }
                    break;
                case 0x2: // slt
                    printOP("slt", &x, &y);
                    break;
                case 0x3: // sltu
                    printOP("sltu", &x, &y);
                    break;
            }

            printREG(rm.instr.rd, &x, &y);
            printREG(rm.instr.rs1, &x, &y);
            printREG(rm.instr.rs2, &x, &y);
          
            vram_adr(NTADR_A(x, ++y));
            vram_put(' ');
            break;
        case 0x13:
            switch (rm.instr.funct3) {
                case 0x0: // addi
                    printOP("addi", &x, &y);

                    addImm16toU32(&rm.regs[rm.instr.rd], rm.instr.imm.b);
                    break;
                case 0x4: // xori
                    printOP("xori", &x, &y);
                    break;
                case 0x6: // ori
                    printOP("ori", &x, &y);
                    break;
                case 0x7: // andi
                    printOP("andi", &x, &y);
                    break;
                case 0x1: // slli
                    printOP("slli", &x, &y);
                    break;
                case 0x5: // srli/srai
                    if (rm.instr.funct7 == 0x00) { // srli
                        printOP("srli", &x, &y);
                    } else if (rm.instr.funct7 == 0x20) { // srai
                        printOP("srai", &x, &y);
                    }
                    break;
                case 0x2: // slti
                    printOP("slti", &x, &y);
                    break;
                case 0x3: // sltiu
                    printOP("sltiu", &x, &y);
                    break;
                default:
                    break;
            }
            
            printREG(rm.instr.rd, &x, &y);
            printREG(rm.instr.rs1, &x, &y);
            printREG('i', &x, &y);

            vram_adr(NTADR_A(x, ++y));
            vram_put(' ');
            break;
        case 0x03: // Load instructions
            break;
        
    }
}


static void __fastcall__ addU32toU32(u32 *dst, const u32 *src) {
    unsigned int i, carry = 0;

    for (i = 0; i < 4; ++i) {
        unsigned int sum = (unsigned int)dst->b[i] + (unsigned int)src->b[i] + carry;
        dst->b[i] = (unsigned char)(sum & 0xFF);
        carry = (sum >> 8) & 0x1;  
    }
}

static void __fastcall__ addImm16toU32(u32 *dst, const unsigned char imm_bytes[2]) {
    /* Build 4-byte sign-extended immediate */
    u32 imm4;
    imm4.b[0] = imm_bytes[0];      // low
    imm4.b[1] = imm_bytes[1];      // high (bit11 is sign)
    /* sign extend byte 2..3 */
    if (imm_bytes[1] & 0x80) {     // if imm12 sign bit set (we earlier sign-extended to imm[1] byte)
        imm4.b[2] = 0xFF;
        imm4.b[3] = 0xFF;
    } else {
        imm4.b[2] = 0x00;
        imm4.b[3] = 0x00;
    }

    addU32toU32(dst, &imm4);
}

static void __fastcall__ printOP(const unsigned char* op, unsigned char* x, unsigned char* y) {
    unsigned char i = 0;
    
    vram_adr(NTADR_A(*x, ++*y));
    while (op[i]) {
        vram_put(op[i]);
        ++i;
    }
}

static void __fastcall__ printREG(const unsigned char reg, unsigned char* x, unsigned char* y) {
    vram_adr(NTADR_A(*x, ++*y));
    if (reg == 'i') {
        vram_put('i');
        vram_put(':');
        vram_put(rm.instr.imm.b[0] + '0');
    } else {
        vram_put('x');
        vram_put(reg + '0');
        vram_put(':');
        vram_put(rm.regs[reg].b[0] + '0');
    }
}
