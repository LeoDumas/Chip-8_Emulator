# Chip-8 Emulator

A simple Chip-8 emulator written in C++.

## Description

This project demonstrates a minimal Chip-8 emulator. It includes:

- Loading a character set (the built-in font sprites for digits and letters).
- Loading ROM files (`.ch8`) from disk into memory.
- A random number generator (`RNG()`).
- Core opcode implementations.
- A 16-level stack for subroutine calls.

The goal of this project is to provide a foundation for emulating and experimenting with classic Chip-8 games and demos and to understand how emulation works for future projects.

## Features

- **LoadCharSet**: Loads the 16 built-in Chip-8 characters into memory at address `0x50`.
- **LoadRom**: Loads a `.ch8` file into memory at the default `START_ADDRESS` (`0x200`).
- **RNG**: Generates a uniform random byte (0–255) for the `RND Vx, byte` instruction.
- **Stack**: 16-level stack for subroutine calls and returns.
- **Implemented Opcodes**:
  - `00E0` (CLS)
  - `00EE` (RET)
  - `1nnn` (JP addr)
  - `2nnn` (CALL addr)
  - `3xkk` (SE Vx, byte)
  - `4xkk` (SNE Vx, byte)
  - `5xy0` (SE Vx, Vy)
  - `6xkk` (LD Vx, byte)
  - `7xkk` (ADD Vx, byte)
  - `8xy0` (LD Vx, Vy)
  - `8xy1` (OR Vx, Vy)
  - `8xy2` (AND Vx, Vy)
  - `8xy3` (XOR Vx, Vy)
  - `8xy4` (ADD Vx, Vy)
  - `8xy5` (SUB Vx, Vy)
  - `8xy6` (SHR Vx)
  - `8xy7` (SUBN Vx, Vy)
  - `8xyE` (SHL Vx)
  - `9xy0` (SNE Vx, Vy)
  - `Annn` (LD I, addr)
  - `Bnnn` (JP V0, addr)
  - `Cxkk` (RND Vx, byte)
  - `Dxyn` (DRW Vx, Vy, nibble)
  - `Ex9E` (SKP Vx)
  - `ExA1` (SKNP Vx)
  - `Fx07` (LD Vx, DT)
  - `Fx0A` (LD Vx, K)
  - `Fx15` (LD DT, Vx)
  - `Fx18` (LD ST, Vx)
  - `Fx1E` (ADD I, Vx)
  - `Fx29` (LD F, Vx)
  - `Fx33` (LD B, Vx)
  - `Fx55` (LD [I], Vx)
  - `Fx65` (LD Vx, [I])

## Prerequisites

- A C++ compiler that supports C++17 or later (e.g., `g++` or `clang++`).
- CMake (optional, to use the provided `CMakeLists.txt`).

## Getting Started

1. **Clone the Repository**
   ```bash
   git clone https://github.com/LeoDumas/Chip-8_Emulator.git
   cd Chip-8_Emulator
   ```

2. **Build**
   - **Using CMake**:
     ```bash
     mkdir build && cd build
     cmake ..
     make
     ```
   - **Direct Compilation**:
     ```bash
     g++ -std=c++17 main.cpp -o chip8_emulator
     ```

3. **Run**
   ```bash
   ./chip8_emulator
   ```

## ROMs

In the sample code, the `Clock_Program_[Bill_Fisher_1981].ch8` file is loaded by default.

> This ROM collection is from [kripod/chip8-roms](https://github.com/kripod/chip8-roms).

Feel free to place other Chip-8 ROM files in the project’s folder (or any directory) and load them via `LoadRom()`.

## Memory Layout

- **Start Address** (`0x200`): Default load address for most Chip-8 programs.
- **Alternate Start** (`0x600`): Some ETI 660 programs expect to begin at `0x600`.
- **Fontset Address** (`0x50`): Where the 80-byte fontset is loaded.

## Missing Functions (To Be Implemented)

Those are the missing functions to be added in the main loop:

- **`EmulateCycle()`**

- **`UpdateTimers()`**
  - Decrement `delayTimer` and `soundTimer` at a 60 Hz rate (if non-zero).

- **`DrawGraphics()`**
  - Render the `video[]` to the screen, with SDL2.

- **`HandleInput()`**
  - Update the `keypad[16]` array based on host computer.

- **`main()`**
  - Main loop function

## Project Structure

```plaintext
.
├── CMakeLists.txt        # CMake build configuration
├── Clock_Program_[Bill_Fisher_1981].ch8
├── main.cpp              # Entry point; currently only loads charset and ROM
├── Chip8.cpp/.hpp       # (not present) Move your class implementation here
├── resources.md          # Additional documentation or notes
└── README.md             # This file
```
