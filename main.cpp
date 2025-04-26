#include <cstdint>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cstring>
#include <chrono>
#include <SDL2/SDL.h>
#include <unordered_map>

// Size of the screen
constexpr int VIDEO_WIDTH = 64;
constexpr int VIDEO_HEIGHT = 32;

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

class Chip8
{
    using OpHandler = void (Chip8::*)();

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

    // First entries from 0x0 to 0xF
    OpHandler primary[16];

    // Opcode that need sub-decoding not only their first digit
    OpHandler table0[16];
    OpHandler table8[16];
    OpHandler tableE[16];
    OpHandler tableF[16];

public:
    Chip8()
        // Constructor
        : index(0), PC(START_ADDRESS), SP(0), delayTimer(0), soundTimer(0), opcode(0)
    {

        // primary
        primary[0x0] = &Chip8::decode_0xxx;
        primary[0x1] = &Chip8::OP_1nnn;
        primary[0x2] = &Chip8::OP_2nnn;
        primary[0x3] = &Chip8::OP_3xkk;
        primary[0x4] = &Chip8::OP_4xkk;
        primary[0x5] = &Chip8::OP_5xy0;
        primary[0x6] = &Chip8::OP_6xkk;
        primary[0x7] = &Chip8::OP_7xkk;
        primary[0x8] = &Chip8::decode_8xxx;
        primary[0x9] = &Chip8::OP_9xy0;
        primary[0xA] = &Chip8::OP_Annn;
        primary[0xB] = &Chip8::OP_Bnnn;
        primary[0xC] = &Chip8::OP_Cxkk;
        primary[0xD] = &Chip8::OP_Dxyn;
        primary[0xE] = &Chip8::decode_Exxx;
        primary[0xF] = &Chip8::decode_Fxxx;

        // table0
        table0[0x0] = &Chip8::OP_00E0;
        table0[0xE] = &Chip8::OP_00EE;

        // 0x8 family
        table8[0x0] = &Chip8::OP_8xy0;
        table8[0x1] = &Chip8::OP_8xy1;
        table8[0x2] = &Chip8::OP_8xy2;
        table8[0x3] = &Chip8::OP_8xy3;
        table8[0x4] = &Chip8::OP_8xy4;
        table8[0x5] = &Chip8::OP_8xy5;
        table8[0x6] = &Chip8::OP_8xy6;
        table8[0x7] = &Chip8::OP_8xy7;
        table8[0xE] = &Chip8::OP_8xyE;

        // 0xE family
        tableE[0x9] = &Chip8::OP_Ex9E;
        tableE[0xA] = &Chip8::OP_ExA1;

        // 0xF family
        tableF[0x07] = &Chip8::OP_Fx07;
        tableF[0x0A] = &Chip8::OP_Fx0A;
        tableF[0x15] = &Chip8::OP_Fx15;
        tableF[0x18] = &Chip8::OP_Fx18;
        tableF[0x1E] = &Chip8::OP_Fx1E;
        tableF[0x29] = &Chip8::OP_Fx29;
        tableF[0x33] = &Chip8::OP_Fx33;
        tableF[0x55] = &Chip8::OP_Fx55;
        tableF[0x65] = &Chip8::OP_Fx65;
    }

    inline void decode_0xxx() { (this->*table0[opcode & 0x000Fu])(); }
    inline void decode_8xxx() { (this->*table8[opcode & 0x000Fu])(); }
    inline void decode_Exxx() { (this->*tableE[(opcode & 0x00F0u) >> 4u])(); }
    inline void decode_Fxxx() { (this->*tableF[opcode & 0x00FFu])(); }

    // Getter for video an keypad to acces them in the main function
    uint32_t *GetVideo() { return video; }
    uint8_t *GetKeypad() { return keypad; }

    void LoadCharSet()
    {
        for (unsigned int i = 0; i < CHAR_SIZE; ++i)
        {
            memory[CHAR_START_ADDRESS + i] = charset[i];
        }
    }

    void LoadRom(const std::string &fileName)
    {
        std::cout << "Loading ROM: " << fileName << std::endl;
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            std::streamsize size = file.tellg();
            char *buffer = new char[size];

            file.seekg(0, std::ios::beg);
            file.read(buffer, size);
            file.close();

            // Fill the Chip8's memory starting at START_ADDRESS.
            for (std::streamsize i = 0; i < size; ++i)
            {
                memory[START_ADDRESS + i] = buffer[i];
            }

            delete[] buffer;
        }
        else
        {
            std::cout << "Can't read the file" << std::endl;
        }
    }

    // By default from 0 to 255
    uint8_t RNG()
    {
        static std::mt19937 gen{std::random_device{}()};
        static std::uniform_int_distribution<uint8_t> dist(0, 255);
        return dist(gen);
    }

    // Instructions definition
    // All based on http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.1

    // CLS
    void OP_00E0()
    {
        memset(video, 0, sizeof(video));
    }

    // RET
    void OP_00EE()
    {
        --SP;
        PC = stack[SP];
    }

    // JP addr
    void OP_1nnn()
    {
        uint16_t addr = opcode & 0x0FFFu; // Extracts the 12-bit address (nnn) from a 16-bit opcode
        PC = addr;
    }

    // CALL addr
    void OP_2nnn()
    {
        uint16_t addr = opcode & 0x0FFFu; // Extract nnn from opcode
        stack[SP] = PC;
        ++SP;
        PC = addr;
    }

    // SE Vx, byte
    void OP_3xkk()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u; // Extract Vx from opcode (bit 8 to 11)
        uint8_t byte = (opcode & 0x00FFu);     // Extract kk (byte) from opcode (bit 0 to 7)

        // The interpreter compares register Vx to kk
        if (registers[Vx] == byte)
        {
            PC += 2;
        }
    }

    // SNE Vx, byte
    void OP_4xkk()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u; // Extract Vx from opcode (bit 8 to 11) and move it's bit to the right 8 times for comparison
        uint8_t byte = (opcode & 0x00FFu);     // Extract kk (byte) from opcode (bit 0 to 7)

        if (registers[Vx] != byte)
        {
            PC += 2;
        }
    }

    // SE Vx, Vy
    void OP_5xy0()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u; // Vx bit 8 to 11
        uint8_t Vy = (opcode & 0x00F0u) >> 4u; // Vy bit 4 to 7

        if (registers[Vx] == registers[Vy])
        {
            PC += 2;
        }
    }

    // LD Vx, byte
    void OP_6xkk()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = (opcode & 0x00FFu);

        registers[Vx] = byte;
    }

    // ADD Vx, byte
    void OP_7xkk()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = (opcode & 0x00FFu);

        registers[Vx] += byte;
    }

    // LD Vx, Vy
    void OP_8xy0()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] = registers[Vy];
    }

    // OR Vx, Vy
    void OP_8xy1()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] |= registers[Vy];
    }

    // AND Vx, Vy
    void OP_8xy2()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] &= registers[Vy];
    }

    // XOR Vx, Vy
    void OP_8xy3()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] ^= registers[Vy];
    }

    // ADD Vx, Vy
    void OP_8xy4()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        // Vf = 0xF

        uint16_t res = ((registers[Vx] + registers[Vy]));

        (res > 0x00FFu) ? registers[0xF] = 1 : registers[0xF] = 0;

        registers[Vx] = (static_cast<uint8_t>(res));
    }

    // SUB Vx, Vy
    void OP_8xy5()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        // Vf = 0xF

        registers[0xF] = ((registers[Vx] > registers[Vy]) ? registers[0xF] = 1 : registers[0xF] = 0);
        registers[Vx] = static_cast<uint8_t>(registers[Vx] - registers[Vy]);
    }

    // SHR Vx {, Vy}
    void OP_8xy6()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        // If LSB[Vx] = 1 -> Vf = 1 and if LSB[Vx] = 0 -> Vf = 0
        // Thus Vf = LSB[Vx]
        registers[0xF] = (registers[Vx] & 0x1u);
        registers[Vx] = registers[Vx] >> 1u;
    }

    // SUBN Vx, Vy
    void OP_8xy7()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[0xF] = (registers[Vy] > registers[Vx] ? 1 : 0);
        registers[Vx] -= registers[Vy];
    }

    // SHL Vx {, Vy}
    void OP_8xyE()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        registers[0xF] = (registers[Vx] & 0x1u);
        registers[Vx] = registers[Vx] << 1u;
    }

    // SNE Vx, Vy
    void OP_9xy0()
    {
        uint8_t Vx = (opcode & 0X0F00u) >> 8u;
        uint8_t Vy = (opcode & 0X00F0u) >> 4u;

        if (registers[Vx] != registers[Vy])
        {
            PC += 2;
        }
    }

    // LD I, addr ; I = index; addr = nnn
    void OP_Annn()
    {
        //                              addr or nnn
        index = static_cast<uint16_t>(opcode & 0x0FFFu);
    }

    // JP V0, addr
    void OP_Bnnn()
    {
        //                          addr or nnn         V0
        PC = static_cast<uint16_t>((opcode & 0x0FFFu) + registers[0]);
    }

    // RND Vx, byte
    // rand number 0 -> 255
    void OP_Cxkk()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = (opcode & 0x00FFu);

        registers[Vx] = (byte & RNG());
    }

    // DRW Vx, Vy, nibble
    void OP_Dxyn()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        uint8_t nibble = (opcode & 0x000Fu);

        uint8_t coordX = registers[Vx] % VIDEO_WIDTH;
        uint8_t coordY = registers[Vy] % VIDEO_HEIGHT;
        registers[0xF] = 0;

        for (uint8_t row = 0; row < nibble; ++row)
        {
            uint8_t spriteByte = memory[index + row];
            for (uint8_t bit = 0; bit < 8u; ++bit)
            {
                uint8_t pixelBit = (spriteByte >> (7 - bit)) & 1;
                uint16_t x = (coordX + bit) % VIDEO_WIDTH;
                uint16_t y = (coordY + row) % VIDEO_HEIGHT;
                uint32_t &screenPixel = video[y * VIDEO_WIDTH + x];

                if (pixelBit)
                {
                    if (screenPixel == 1)
                    {
                        registers[0xF] = 1; // If collision detected
                    }
                    screenPixel ^= 1;
                }
            }
        }
    }

    // SKP Vx
    void OP_Ex9E()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vx_key = registers[Vx];
        if (keypad[Vx_key])
        {
            PC += 2;
        }
    }

    // SKNP Vx
    void OP_ExA1()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vx_key = registers[Vx];
        if (!keypad[Vx_key])
        {
            PC += 2;
        }
    }

    // LD Vx, DT
    void OP_Fx07()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        registers[Vx] = delayTimer;
    }

    // LD Vx, K
    void OP_Fx0A()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        bool keyPressed = false;

        for (int i = 0; i < 16; ++i)
        {
            if (keypad[i])
            {
                registers[Vx] = i;
                keyPressed = true;
                break;
            }
        }
        if (!keyPressed)
        {
            PC -= 2;
        }
    }

    // LD DT, Vx
    void OP_Fx15()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        delayTimer = registers[Vx];
    }

    // LD ST, Vx
    void OP_Fx18()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        soundTimer = registers[Vx];
    }

    // ADD I, Vx
    void OP_Fx1E()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        index += registers[Vx];
    }

    // LD F, Vx
    void OP_Fx29()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        // Sprites are 5 bytes long
        index = CHAR_START_ADDRESS + (5 * registers[Vx]);
    }

    // LD B, Vx
    void OP_Fx33()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t val = registers[Vx];

        memory[index + 2] = val % 10;
        val /= 10;

        memory[index + 1] = val % 10;
        val /= 10;

        memory[index] = val % 10;
    }

    // LD [I], Vx
    void OP_Fx55()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        for (uint8_t i = 0; i <= Vx; ++i)
        {
            memory[index + i] = registers[i];
        }
    }

    // LD Vx, [I]
    void OP_Fx65()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        for (uint8_t i = 0; i <= Vx; ++i)
        {
            registers[i] = memory[index + i];
        }
    }

    void Cycle()
    {
        // Fetch
        opcode = (static_cast<uint16_t>(memory[PC]) << 8) | static_cast<uint16_t>(memory[PC + 1]);

        // Instruction are 2 bytes long, we need to move the PC forward by 2 for the next opcode
        PC += 2;

        std::cout << "Executing opcode: 0x" << std::hex << opcode << std::dec << std::endl;

        // Decode and execute
        (this->*(primary[(opcode & 0xF000u) >> 12u]))();

        // Decrement the delay timer if it's been set
        if (delayTimer > 0)
        {
            --delayTimer;
        }

        // Decrement the sound timer if it's been set
        if (soundTimer > 0)
        {
            --soundTimer;
        }
    }
};

class Platform
{
public:
    Platform(char const *title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
    {
        SDL_Init(SDL_INIT_VIDEO);

        window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        texture = SDL_CreateTexture(
            renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
    }

    ~Platform()
    {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Update(void const *buffer, int pitch)
    {
        // Create a temporary buffer for proper colors
        uint32_t pixels[VIDEO_WIDTH * VIDEO_HEIGHT];
        const uint32_t *videoBuffer = static_cast<const uint32_t *>(buffer);

        // Convert 0 & 1 values to black & white colors
        for (int i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++)
        {
            pixels[i] = videoBuffer[i] ? 0xFFFFFFFF : 0x000000FF; // White or black with alpha
        }

        SDL_UpdateTexture(texture, nullptr, pixels, pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
    bool ProcessInput(uint8_t *keys)
    {
        bool quit = false;
        SDL_Event event;

        // Map physical keys to chip-8 keypad
        static const std::unordered_map<SDL_Keycode, uint8_t> keyMap = {
            {SDLK_x, 0x0},
            {SDLK_1, 0x1},
            {SDLK_2, 0x2},
            {SDLK_3, 0x3},
            {SDLK_a, 0x4},
            {SDLK_z, 0x5},
            {SDLK_e, 0x6},
            {SDLK_q, 0x7},
            {SDLK_s, 0x8},
            {SDLK_d, 0x9},
            {SDLK_w, 0xA},
            {SDLK_c, 0xB},
            {SDLK_4, 0xC},
            {SDLK_r, 0xD},
            {SDLK_f, 0xE},
            {SDLK_v, 0xF},
        };

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                // Escape still quits
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true;
                    break;
                }
                auto it = keyMap.find(event.key.keysym.sym);
                if (it != keyMap.end())
                {
                    // press = 1, release = 0
                    keys[it->second] = (event.type == SDL_KEYDOWN);
                }
            }
            break;
            }
        }

        return quit;
    }

private:
    SDL_Window *window{};
    SDL_Renderer *renderer{};
    SDL_Texture *texture{};
};

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay(ms)> <ROM>\n";
        return EXIT_FAILURE;
    }

    int videoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    const char *romFilename = argv[3];

    Platform platform(
        "CHIP-8 Emulator",
        VIDEO_WIDTH * videoScale,
        VIDEO_HEIGHT * videoScale,
        VIDEO_WIDTH,
        VIDEO_HEIGHT);

    Chip8 chip8;
    chip8.LoadCharSet();
    chip8.LoadRom(romFilename);

    int videoPitch = sizeof(uint32_t) * VIDEO_WIDTH;
    auto lastCycle = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit)
    {
        quit = platform.ProcessInput(chip8.GetKeypad());

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(now - lastCycle).count();

        if (dt > cycleDelay)
        {
            lastCycle = now;
            chip8.Cycle();
            platform.Update(chip8.GetVideo(), videoPitch);
        }
    }

    return EXIT_SUCCESS;
}
