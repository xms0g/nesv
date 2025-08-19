# nesv
**nesv** is a toy RISC-V emulator that runs entirely on the **Nintendo Entertainment System (NES)**, written in C (compiled with [cc65](https://cc65.github.io/)), demonstrates that a modern 32-bit ISA can be emulated on 8-bit retro hardware.

<img src="assets/nesv.png" alt="image" width="700" height="auto">

## Features
- Only supports **RV32I**; `branch`, `load/store`, and `arithmetic/logical` instructions implemented (no compressed, no floating point, no atomics). 

- Runs on a stock NES / emulator with a standard `.nes` ROM.

- Debug output shown on NES screen (registers, PC dumps) 

- No support for interrupts, traps, or a full RISC-V environment.

## Building
To build and run, you’ll need the following tools:

- [`cc65`](https://cc65.github.io/) cross development toolchain
- `NES` emulator (e.g., FCEUX)

## License
This project is licensed under the BSD 3-Clause License. See the LICENSE file for details.
