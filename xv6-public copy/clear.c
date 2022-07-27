#include "user.h"

int main(){
	// These are ANSI escape codes.
	// \033 stands for ESC
	// ESC[ is a kind of escape sequence called Control Sequence Introducer (CSI).
	// \033[H and \033[2J are CSI codes
	// \033[H moves the cursor to the top left corner of the screen (ie, the first column of the first row in the screen).
	// \033[J clears the part of the screen from the cursor to the end of the screen.
	printf(1,"\33[H\33[2J");
	exit();
}
