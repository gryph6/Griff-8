#include "Chip8.h"
#include "Chip8_fontset.h"

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

void Chip8::initialize() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
    
    // Clear display
    clearDisplay();

    // Clear stack
    for (int i = 0; i < 16; ++i) {
        stack[i] = 0;
    }

    // Clear registers
    for (int i = 0; i < 16; ++i) {
        V[i] = 0;
    }

    // Clear memory
    for (int i = 0; i < 4096; ++i) {
        memory[i] = 0;
    }

    // Load fontset
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }

    // Reset timers
    delay_timer = 0;
    sound_timer = 0;
}

bool Chip8::loadProgram(std::string path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        return false;
    }

    // Load program into memory starting at 0x0200
    for (int i = 0; i < size; ++i) {
        memory[i + 0x200] = buffer[i];
    }

    return true;
}

void Chip8::emulateCycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xF000) {
        case 0x1000: // 0x1NNN: Jump to address NNN.
            sp = opcode & 0x0FFF;
            break;
        case 0x2000: // 0x2NNN: Call subroutine at address NNN. Store current stack pointer in stack, move stack pointer to address NNN.
            stack[sp++] = sp;
            sp = opcode & 0x0FFF;
            break;
        case 0x3000: // 0x3XNN: Skip next instruction if VX == NN
            X = (opcode & 0x0F00) >> 8;
            if (V[X] == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0x4000: // 0x4XNN: Skip next instruction if VX != NN
            X = (opcode & 0x0F00) >> 8;
            if (V[X] != (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0x5000: // 0x5XY0: Skip next instruction if VX == VY
            X = (opcode & 0x0F00) >> 8;
            Y = (opcode & 0x00F0) >> 8;
            if (V[X] == V[Y]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0x6000: // 0x6XNN: Set V[X] to NN
            X = (opcode & 0x0F00) >> 8;
            V[X] = opcode & 0x00FF;
            pc += 2;
            break;
        case 0xA000: // 0xANNN: Sets I to address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000: // 0x00E0: Clear screen
                    clearDisplay();
                    pc += 2;
                    break;
                case 0x000E: // 0x00EE: Return from subroutine
                    sp = stack[--sp];
                    break;
            }
        default:
            std::cout << "ERROR: Unknown opcode: " << opcode << std::endl;
            break;
    }
}

void Chip8::clearDisplay() {
    for (int i = 0; i < 64 * 32; ++i) {
        gfx[i] = 0;
    }
}

void Chip8::updateTimers() {
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            std::cout << "HARDWARE BEEP" << std::endl;
        }
        --sound_timer;
    }
}