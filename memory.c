#include <stdio.h>
#include <stdlib.h>
#include "common.h"

unsigned char *PRG;
unsigned char RAM[0x800];

void map_program_data(unsigned char *prg_data) {
	PRG = prg_data;
}

unsigned char read_memory(unsigned short address) {
	if (address > 0x7FFF) {
		printf("  read_memory  %04X -> PRG %04X -> %02X\n", address, address & 0x3FFF, PRG[address & 0x3FFF]);
		return PRG[address & 0x3FFF];
	}
	if (address < 0x2000) {
		printf("  read_memory  %04X -> RAM %04X -> %02X\n", address, address & 0x07FF, RAM[address & 0x07FF]);
		return RAM[address & 0x07FF];
	}
	if (address < 0x4000) {
		unsigned char ppu_result = ppu_read(address);
		printf("  read_memory  %04X -> \033[1;34mPPU\033[0m %X -> %02X\n", address, address & 0x0007, ppu_result);
		return ppu_result;
	}
	if (address == 0x4016 || address == 0x4017) {
		printf("  read_memory  %04X -> \033[1;45mCTRL\033[0m %04X -> %02X\n", address, address & 0x3FFF, PRG[address & 0x3FFF]);
getchar();
		return 0;
	}
#undef printf
	printf("  read_memory  %04X ERR\n", address);
#define printf 0&&printf
	exit(2);
}

void write_memory(unsigned short address, unsigned char data) {
	if (address < 0x2000) {
		printf("  write_memory %04X -> RAM %04X -> %02X\n", address, address & 0x07FF, data);
		RAM[address & 0x07FF] = data;
		return;
	}
	if (address < 0x4000) {
		printf("  write_memory %04X -> \033[1;34mPPU\033[0m %X -> %02X\n", address, address & 0x0007, data);
		ppu_write(address, data);
		return;
	}
	if (address == 0x4014) {
		printf("  write_memory %04X -> \033[1;35mOAMDMA\033[0m %04X -> %02X\n", address, address & 0x000F, data);
		//ppu_write(address & 0x00FF, data);
		return;
	}
	if (address >= 0x4000 && address <= 0x4017) {
		printf("  write_memory %04X -> \033[1;45mCTRL\033[0m %04X -> %02X\n", address, address & 0x000F, data);
getchar();
		return;
	}
	printf("  write_memory %04X -> %02X ERR\n", address, data);
	exit(3);
}

