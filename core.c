#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

int main(int argc, char *argv[]) {
	fprintf(stdout, "NES emulator\n");
	load_ROM(argc > 1 ? argv[1] : "donkeykong.nes");
	io_setup();
	ppu_setup();

	while (true) {
		ppu_run();
		ppu_run();
		cpu_exec();
		ppu_run();
		ppu_run();
		ppu_run();
		cpu_exec();
		ppu_run();
		ppu_run();
		ppu_run();
		cpu_exec();
		ppu_run();
		ppu_run();
		ppu_run();
		cpu_exec();
		ppu_run();
	}
}

