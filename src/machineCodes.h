// li t1, 1
// li t2, 2
// add t0, t1, t2 
static const unsigned char add[] = {
	 // li t1, 1     → addi t1, x0, 1   → 0x00100313
    0x13, 0x03, 0x10, 0x00,

    // li t2, 2     → addi t2, x0, 2    → 0x00200393
    0x93, 0x03, 0x20, 0x00,

    // add t0, t1, t2 → add x5, x6, x7  → 0x007302b3
    0xb3, 0x02, 0x73, 0x00
};

// li t1, 5
// li t2, 3
// sub t0, t1, t2 
static const unsigned char sub[] = {
	 // li t1, 5     → addi t1, x0, 5   → 0x00500313
    0x13, 0x03, 0x50, 0x00,

    // li t2, 3     → addi t2, x0, 3    → 0x00300393
    0x93, 0x03, 0x30, 0x00,

    // sub t0, t1, t2 → sub x5, x6, x7  → 0x407302b3
    0xb3, 0x02, 0x73, 0x40
};
// li t1, 5
// li t2, 3
// sll t0, t1, t2 
static const unsigned char sll[] = {
	 // li t1, 5     → addi t1, x0, 5   → 0x00500313
    0x13, 0x03, 0x50, 0x00,

    // li t2, 3     → addi t2, x0, 3    → 0x00300393
    0x93, 0x03, 0x30, 0x00,

    // sll t0, t1, t2 → sll x5, x6, x7  → 0x007312b3
    0xb3, 0x12, 0x73, 0x00
};