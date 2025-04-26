# CHIP-8 Emulator

A simple, SDL2‑powered Chip-8 emulator written in C++.

⚠️ Not all programs works for the moments. (Games doesn't work)

---

## Table of Contents

- [CHIP-8 Emulator](#chip-8-emulator)
  - [Table of Contents](#table-of-contents)
  - [Project Overview](#project-overview)
  - [Features](#features)
  - [Prerequisites](#prerequisites)
  - [Building](#building)
    - [Using CMake (recommended)](#using-cmake-recommended)
    - [Direct compilation (single file)](#direct-compilation-single-file)
  - [Usage](#usage)
  - [Programs / ROMs](#programs--roms)
  - [Memory Layout](#memory-layout)
  - [Project Structure](#project-structure)

---

## Project Overview

This emulator implements the classic Chip-8, allowing you to load and run `.ch8` ROMs using SDL2 for graphics and input. It demonstrates:

[video_demo](https://github.com/user-attachments/assets/35eb31d3-71a1-4da8-b64c-5d73f72dd290)

- **Opcode decoding and execution** for the full Chip-8 instruction set.
- **Graphics rendering** at 64×32 pixels, scaled to any window size.
- **Keyboard input mapping** between modern keyboards and the 16-key Chip‑8 keypad.
- **Timers** and **sound** emulation (via decrementing timers).

---

## Features

- **Full opcode support**:
  - Display: `00E0`, `Dxyn`
  - Flow control: `1nnn`, `2nnn`, `00EE`
  - Conditional skips: `3xkk`, `4xkk`, `5xy0`, `9xy0`, `Ex9E`, `ExA1`
  - Registers and math: `6xkk`, `7xkk`, `8xy0`–`8xyE`
  - Index register: `Annn`, `Fx1E`
  - I/O: `Fx07`, `Fx0A`, `Fx15`, `Fx18`, `Fx29`, `Fx33`, `Fx55`, `Fx65`
  - Random: `Cxkk`
- **Fontset loading** at address `0x50` for hexadecimal characters (0–F).
- **Configurable start address** (`0x200` by default, `0x600` optional for ETI 660 programs).
- **16-level stack** for subroutines.
- **SDL2** integration for window, rendering, and event handling.
- **Command-line options** for pixel scaling and cycle delay.

---

## Prerequisites

- **C++17** compiler (e.g., `g++`, `clang++`)
- **SDL2** development libraries
- **CMake** (optional)

On Debian/Ubuntu:

```bash
sudo apt-get update && \
  sudo apt-get install build-essential libsdl2-dev cmake
```

---

## Building

### Using CMake (recommended)

```bash
git clone https://github.com/LeoDumas/Chip-8_Emulator.git
cd Chip-8_Emulator
mkdir build && cd build
cmake ..
make
```

### Direct compilation (single file)

```bash
g++ -std=c++17 main.cpp -lSDL2 -O2 -o chip8_emulator
```

---

## Usage

```bash
./chip8_emulator <Scale> <Delay(ms)> <ROM_PATH>
```

- `<Scale>`: Integer scaling factor for the 64×32 display (e.g., `10` for 640×320 window).
- `<Delay(ms)>`: Minimum milliseconds per emulation cycle (e.g., `2` for ~500 cycles/second).
- `<ROM_PATH>`: Path to the `.ch8` ROM file.

**Example:**

```bash
./build/Desktop-Debug/chip_8_emulator 10 1 ./programs/Keypad_Test_\[Hap_2006\].ch8
```

Press **Escape** or close the window to quit.

Key mapping:
```
1 2 3 C    → Chip-8 1 2 3 C
4 5 6 D    → Chip-8 4 5 6 D
7 8 9 E    → Chip-8 7 8 9 E
A 0 B F    → Chip-8 A 0 B F
Mapping uses keys: X,1,2,3,A,Z,E,Q,S,D,W,C,4,R,F,V (azerty)
```

---

## Programs / ROMs

Contains a small collection of test ROMs in the `programs/` folder:

[https://github.com/kripod/chip8-roms/tree/master](https://github.com/kripod/chip8-roms/tree/master)

- `15_Puzzle_[Roger_Ivie].ch8`
- `Airplane.ch8`
- `Clock_Program_[Bill_Fisher_1981].ch8`
- `Fishie_[Hap_2005].ch8`
- `Keypad_Test_[Hap_2006].ch8`
- `Maze_[David_Winter_199x].ch8`
- `Stars_[Sergey_Naydenov_2010].ch8`

---

## Memory Layout

- **Fontset**: loaded at `0x50`, size 80 bytes
- **Program start**: default `0x200` (first 512 bytes reserved)
- **Alternate start**: `0x600` for ETI 660 compatibility

---

## Project Structure

```
.
├── .vscode/               # VSCode workspace settings
├── build/                 # CMake build directory
├── programs/              # Sample Chip-8 ROM files
│   ├── *.ch8
│   └── …
├── CMakeLists.txt         # Build configuration
├── main.cpp               # Emulator implementation
├── README.md              # This file
└── ressources.md          # Additional notes
```