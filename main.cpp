#include "Chip8.h"

int main() {
        // Setup Graphics
	// Setup Input
	
	Chip8 chip8;
	
	chip8.initialize();
	chip8.loadProgram("C:\\game.game");

	// Emulation loop
	while (true) {
		// chip8.emulateCycle();
		//
		// if (chip8.drawFlag) drawGraphics();
		//
		// chip8.setKeys();		
	}	

	return 0;
}
