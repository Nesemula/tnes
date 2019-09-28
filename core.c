#include <stdio.h>
#include "common.h"

int main(int argc, char *argv[]) {
	load_ROM(argc > 1 ? argv[1] : "donkey_kong.nes");
	initialize_display();
	printf("T-NES\n");
	//for (int i = 0; i < 25; i++)
		//ppu_exec();
#if DBG
	unsigned long long counter = 0;
	while (counter++ < 20) {
#else
	while (1) {
#endif
		ppu_exec();
		ppu_exec();
		ppu_exec();
		cpu_exec();
	}
	getchar();
}

