#define BLACK 0x0f
#define DK_GY 0x00
#define LT_GY 0x10
#define WHITE 0x30

//put all the subsequent global vars into zeropage, to make code faster and shorter
#pragma bss-name (push,"ZEROPAGE")
#pragma data-name (push,"ZEROPAGE")

const unsigned char palette[] = { 
    BLACK, DK_GY, LT_GY, WHITE,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0
}; 
