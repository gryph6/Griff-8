#include <string>

class Chip8 {
public:
	void initialize();
	bool loadProgram(std::string path);
	void emulateCycle();

private:
	void clearDisplay();
	void updateTimers();

	unsigned short opcode;
	unsigned short memory[4096];
	unsigned char V[16];

	unsigned short I;
	unsigned short pc;

	unsigned char gfx[64 * 32];

	unsigned char delay_timer;
	unsigned char sound_timer;

	unsigned short stack[16];
	unsigned short sp;

	unsigned char key[16];

	unsigned char X;
	unsigned char Y;
};
