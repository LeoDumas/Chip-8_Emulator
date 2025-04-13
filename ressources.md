# Ressources for a Chip-8 Emulator

The chip-8 use only 4KB or RAM, from 0x000 to 0xFFF.

First 512 bytes are where the original interpreter is located ,and should not be used

The Registers, with 16 8bit registers registered from V0 to VF (Hex)

**VF** Should not be used because it hold information about the result of operations.

The Program Counter (PC) is a special register that holds the address of the next instruction to execute.

The Stack Pointer (SP) tell us where in the 16-levels of stack our most recent value was placed (i.e, the top)

[Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)

