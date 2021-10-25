#include "Chip8.h"
#include "Chip8_fontset.h"

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>

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
    _draw = false;

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000: // 0x00E0: Clear screen
                    clearDisplay();
                    pc += 2;
                    break;
                case 0x000E: // 0x00EE: Return from subroutine
                    pc = stack[--sp];
                    break;
            }
        case 0x1000: // 0x1NNN: Jump to address NNN.
            pc = opcode & 0x0FFF;
            break;
        case 0x2000: // 0x2NNN: Call subroutine at address NNN. Store current stack pointer in stack, move stack pointer to address NNN.
            stack[sp++] = pc;
            pc = opcode & 0x0FFF;
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
            Y = (opcode & 0x00F0) >> 4;
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
        case 0x7000: // 0x7XNN: Adds NN to V[X] (carry flag unaffected).
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
				case 0x0000: // 0x8XY0: Set V[x] = V[y]
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0001: // 0x8XY1: Set V[x] = V[x] | V[y]
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0002: // 0x8XY2: Set V[x] = V[x] & V[y]
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0003: // 0x8XY3: Set V[x] = V[x] ^ V[y]
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
			    case 0x0004: // 0x8XY4: Set V[x] = V[x] + V[y]. V[0xF] is set if there's overflow, cleared if not.
                    if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
                case 0x0005: // 0x8XY5: Set V[x] = V[x] - V[y]. V[0xF] is clear when there's a borrow, set if not.
                    if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0006: // 0x8XY6: Store LSB of V[X] in V[0xF] and shift V[X] to right by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x0001;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;
                case 0x0007: // 0x8XY7: Set V[x] = V[y] - V[x]. V[0xF] is clear when there's a borrow, set if not.
                    if (V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x00F0) >> 4] -= V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x000E: // 0x8XY6: Store MSB of V[X] in V[0xF] and shift V[X] to left by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x8000;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;
            }
        case 0x9000: // 0x9XY0: Skip next instruction if V[X] != V[Y]
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0xA000: // 0xANNN: Sets I to address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0xB000: // 0xBNNN: Jumps to address NNN plus V[0]
            pc = (opcode & 0x0FFF) + V[0];
            break;
        case 0xC000: // 0xCXNN: Set V[X] equal to rand([0, 255]) & NN
            V[(opcode & 0x0F00) >> 8] = ((unsigned char) (std::rand() * 255)) & (opcode & 0x00FF);
            pc += 2;
            break;
        case 0xD000: { // Draw function.
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            _draw = true;
            pc += 2;
            break;
        }
        case 0xE000:
            switch (opcode & 0x000F) {
				case 0x000E: { // 0xEX9E: skip if key[V[X]] is pressed
                    unsigned char x = V[(opcode & 0x0F00) >> 8];
                    if (key[x]) {
                        pc += 4;
                    } else {
						pc += 2;
                    }
					break;
                }
                case 0x0001: { // 0x0ExA1: skip if key[V[X]] is not pressed
                    unsigned char x = V[(opcode & 0x0F00) >> 8];
                    if (!key[x]) {
                        pc += 4;
                    } else {
						pc += 2;
                    }
                    break;
                }
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