#include <cstdint>
#include <fstream>
#include <string>

const unsigned int START_ADDRESS = 0x200;

class Chip8 {
public:
    Chip8()
        // Constructor
        : index(0), PC(START_ADDRESS), SP(0), delayTimer(0), soundTimer(0), opcode(0){}

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
        }
    }

private:
    uint32_t video[64 * 32]{};
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
    myChip.LoadRom("./Clock Program [Bill Fisher, 1981].ch8");
    return 0;
}
