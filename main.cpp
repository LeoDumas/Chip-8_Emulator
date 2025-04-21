#include <cstdint>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cstring>


const unsigned int START_ADDRESS = 0x200; // Start at this location because the first 512 bytes, from 0x000 to 0x1FF, are where the original interpreter was located, and should not be used by programs.
// We can also use START_ADDRESS = 0x600 for some programs that are intended for the ETI 660 computer
// Found here : http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.1

// Characters declaration
const unsigned int CHAR_SIZE = 80; // 80 correspond to the number of element in the charset (0xF0, 0x90,...)

// Based on http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.3
uint8_t charset[CHAR_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
const unsigned int CHAR_START_ADDRESS = 0X50;

class Chip8 {
public:
    Chip8()
        // Constructor
        : index(0), PC(START_ADDRESS), SP(0), delayTimer(0), soundTimer(0), opcode(0){}

    void LoadCharSet(){
        for (unsigned int i = 0; i < CHAR_SIZE; ++i){
            memory[CHAR_START_ADDRESS + i] = charset[i];
        }
    }

    void LoadRom(const std::string &fileName) {
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::streamsize size = file.tellg();
            char* buffer = new char[size];

            file.seekg(0, std::ios::beg);
            file.read(buffer, size);
            file.close();

            // Fill the Chip8's memory starting at START_ADDRESS.
            for (std::streamsize i = 0; i < size; ++i) {
                memory[START_ADDRESS + i] = buffer[i];
            }

            delete[] buffer;
        }else{
            std::cout << "Can't read the file" << std::endl;
        }
    }

    // By default from 0 to 255
    uint8_t RNG(){
        static std::mt19937 gen{ std::random_device{}() };
        static std::uniform_int_distribution<uint8_t> dist(0,255);
        return dist(gen);
    }

    // Instructions definition
    // All based on http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.1

    // CLS
    void OP_00E0(){
        memset(video, 0, sizeof(video));
    }

    // RET
    void OP_00EE(){
        --SP;
        PC = stack[SP];
    }

    // JP addr
    void OP_1nnn(){
        uint16_t addr = opcode & 0x0FFFu; // Extracts the 12-bit address (nnn) from a 16-bit opcode
        PC = addr;
    }

    // CALL addr
    void OP_2nnn(){
        uint16_t addr = opcode & 0x0FFFu; // Extract nnn from opcode
        stack[SP] = PC;
        ++SP;
        PC = addr;
    }

    // SE Vx, byte
    void OP_3xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u; // Extract Vx from opcode (bit 8 to 11)
        uint8_t byte = (opcode & 0x00FFu); // Extract kk (byte) from opcode (bit 0 to 7)

        // The interpreter compares register Vx to kk
        if(registers[Vx] == byte){
            PC += 2;
        }
    }

    // SNE Vx, byte
    void OP_4xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u; // Extract Vx from opcode (bit 8 to 11) and move it's bit to the right 8 times for comparison
        uint8_t byte = (opcode & 0x00FFu); // Extract kk (byte) from opcode (bit 0 to 7)

        if(registers[Vx] != byte){
            PC += 2;
        }
    }

    // SE Vx, Vy
    void OP_5xy0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u; // Vx bit 8 to 11
        uint8_t Vy = (opcode & 0x00F0u) >> 4u; // Vy bit 4 to 7

        if(registers[Vx] == registers[Vy]){
            PC += 2;
        }
    }

    // LD Vx, byte
    void OP_6xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = (opcode & 0x00FFu);

        registers[Vx] = byte;
    }

    // ADD Vx, byte
    void OP_7xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = (opcode & 0x00FFu);

        registers[Vx] += byte;
    }

    // LD Vx, Vy
    void OP_8xy0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] = registers[Vy];
    }

    // OR Vx, Vy
    void OP_8xy1(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] |= registers[Vy];
    }

    // AND Vx, Vy
    void OP_8xy2(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] &= registers[Vy];
    }

    // XOR Vx, Vy
    void OP_8xy3(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] ^= registers[Vy];
    }

    // ADD Vx, Vy
    void OP_8xy4(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        // Vf = 0xF

        uint16_t res = ((registers[Vx] + registers[Vy]));

        (res > 0x00FFu) ? registers[0xF] = 1 : registers[0xF] = 0;

        registers[Vx] = (static_cast<uint8_t>(res));
    }

    // SUB Vx, Vy
    void OP_8xy5(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        // Vf = 0xF

        registers[0xF] = ((registers[Vx] > registers[Vy]) ? registers[0xF] = 1 : registers[0xF] = 0);
        registers[Vx] = static_cast<uint8_t>(registers[Vx] - registers[Vy]);
    }

    // SHR Vx {, Vy}
    void OP_8xy6(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        // If LSB[Vx] = 1 -> Vf = 1 and if LSB[Vx] = 0 -> Vf = 0
        // Thus Vf = LSB[Vx]
        registers[0xF] = (registers[Vx] & 0x1u);
        registers[Vx] = registers[Vx] >> 1u;
    }

    // SUBN Vx, Vy
    void OP_8xy7(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[0xF] = (registers[Vy] > registers[Vx] ? 1 : 0);
        registers[Vx] -= registers[Vy];
    }

    // SHL Vx {, Vy}
    void OP_8xyE(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        registers[0xF] = (registers[Vx] & 0x1u);
        registers[Vx] = registers[Vx] << 1u;
    }

    // SNE Vx, Vy
    void OP_9xy0(){
        uint8_t Vx = (opcode & 0X0F00u) >> 8u;
        uint8_t Vy = (opcode & 0X00F0u) >> 4u;

        PC += (registers[Vx] != registers[Vy] && 2);
    }

    // LD I, addr ; I = index; addr = nnn
    void OP_Annn(){
        //                              addr or nnn
        index = static_cast<uint16_t>(opcode & 0x0FFFu);
    }

    //JP V0, addr
    void OP_Bnnn(){
        //                          addr or nnn         V0
        PC = static_cast<uint16_t>((opcode & 0x0FFFu)+registers[0]);
    }

    // RND Vx, byte
    // rand number 0 -> 255
    void OP_Cxkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = (opcode & 0x00FFu);

        registers[Vx] = (byte & RNG());
    }

    // DRW Vx, Vy, nibble
    void OP_Dxyn(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        uint8_t nibble = (opcode & 0x000Fu);

        uint8_t coordX = registers[Vx] % VIDEO_WIDTH;
        uint8_t coordY = registers[Vy] % VIDEO_HEIGHT;
        registers[0xF] = 0;

        for(uint8_t row = 0; row < nibble; ++row){
            uint8_t spriteByte = memory[index + row];
            for(uint8_t bit = 0; bit < 8u; ++bit){
                uint8_t pixelBit = (spriteByte >> (7 - bit)) & 1;
                uint16_t x = (coordX + bit) % VIDEO_WIDTH;
                uint16_t y = (coordY + row) % VIDEO_HEIGHT;
                uint32_t &screenPixel = video[y * VIDEO_WIDTH + x];

                if (pixelBit) {
                    if (screenPixel == 1) {
                        registers[0xF] = 1; // collision détectée
                    }
                    screenPixel ^= 1;
                }
            }
        }
    }

    // SKP Vx
    void OP_Ex9E(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vx_key = registers[Vx];
        if(keypad[Vx_key]){
            PC += 2;
        }
    }

    // SKNP Vx
    void OP_ExA1(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vx_key = registers[Vx];
        if(!keypad[Vx_key]){
            PC += 2;
        }
    }

    // LD Vx, DT
    void OP_Fx07(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        registers[Vx] = delayTimer;
    }

    // LD Vx, K
    void OP_Fx0A(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        bool keyPressed = false;

        for(int i = 0; i < 16; ++i){
            if(keypad[i]){
                registers[Vx] = i;
                keyPressed = true;
                break;
            }
        }
        if (!keyPressed){
            PC -= 2;
        }
    }

    // LD DT, Vx
    void OP_Fx15(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        delayTimer = registers[Vx];
    }

    // LD ST, Vx
    void OP_Fx18(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        soundTimer = registers[Vx];
    }

    // ADD I, Vx
    void OP_Fx1E(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        index += registers[Vx];
    }

    // LD F, Vx
    void OP_Fx29(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
                                    // Sprites are 5 bytes long
        index = CHAR_START_ADDRESS + (5 * registers[Vx]);
    }

    // LD B, Vx
    void OP_Fx33(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t val = registers[Vx];

        memory[index + 2] = val % 10;
        val /= 10;

        memory[index + 1] = val % 10;
        val /= 10;

        memory[index] = val % 10;
    }

    // LD [I], Vx
    void OP_Fx55(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        for (uint8_t i = 0; i <= Vx; ++i){
            memory[index + i] = registers[i];
        }
    }

    // LD Vx, [I]
    void OP_Fx65(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        for (uint8_t i = 0; i <= Vx; ++i){
            registers[i] = memory[index + i];
        }

    }

private:
    static constexpr uint8_t VIDEO_WIDTH = 64;
    static constexpr uint8_t VIDEO_HEIGHT = 32;
    uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};
    uint8_t keypad[16]{};
    uint8_t registers[16]{};
    uint8_t memory[4096]{};
    uint16_t index;
    uint16_t PC;
    uint8_t SP;
    uint16_t stack[16]{};
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint16_t opcode;
};

int main(){
    Chip8 myChip;
    myChip.LoadCharSet();
    myChip.LoadRom("../../Clock_Program_[Bill_Fisher_1981].ch8");
    return 0;
}
