#include <string>

class Chip8 {
public:
	void initialize();
	bool loadProgram(std::string path);
	void emulateCycle();
	bool shouldDraw();
	void updateTimers();
	void setKey(unsigned int key_index);
	void unsetKey(unsigned int key_index);
	unsigned char* getGraphics();

private:
	void handleUnknownCode();
	void clearDisplay();

	unsigned short opcode;

	unsigned char memory[4096];
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

	bool _draw = false;
};
