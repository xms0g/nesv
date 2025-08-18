# nesv
**nesv** is a toy RISC-V emulator that runs entirely on the **Nintendo Entertainment System (NES)**, demonstrates that a modern 32-bit ISA can be emulated on 8-bit retro hardware.

## Features

- Emulates the **RISC-V RV32I base instruction set**  
- `Branch`, `load/store`, and `arithmetic/logical` instructions implemented  
- Runs on a stock NES / emulator with a standard `.nes` ROM  
- Debug output shown on NES screen (registers and PC dumps)  
- Simple test programs in RISC-V machine code can be loaded and executed  

## Building
To build and run, you’ll need the following tools:

- [`cc65`](https://cc65.github.io/)  cross development package
- `NES` emulator (e.g., FCEUX)

## License
This project is licensed under the BSD 3-Clause License. See the LICENSE file for details.
