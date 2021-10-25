#include <string>
#include <ifstream>


class Chip8 {
	void initialize() {
		pc = 0x200;
		opcode = 0;
		I = 0;
		sp = 0;
		
		// Clear display
		for (int i = 0; i < 64 * 32; ++i) {
			gfx[i] = 0;
		}

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
	}

	bool loadProgram(std::string path) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<char> buffer(size);
		if (!file.read(buffer.data(), size))
		{
			return false;
		}

		for (int i = 0; i < size; ++i) {
			memory[i + 0x200] = buffer[i];
		}

		return true;
	}

	void emulateCycle() {
		opcode = memory[pc] << 8 | memory[pc + 1];

		switch (opcode & 0xF000) {
			case 0xA000: // ANNN: Sets I to address NNN
				I = opcode & 0x0FFF;
				pc += 2;
				break;
			default:
				std::cout << "ERROR: Unknown opcode: " << opcode << std::endl;
		}
	}

	void updateTimers() {
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

	private:
		unsigned short opcode;
		unsigned short memory[4096];
		unsigned char V[16];

		unsigned short I;
		unsigned short pc;

		unsigned char gfx[64 * 32]

		unsigned char delay_timer;
		unsigned char sound_timer;

		unsigned short stack[16];
		unsigned short sp;

		unsigned char key[16];
};
