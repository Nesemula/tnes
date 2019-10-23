#include <stdio.h>
#include <stdlib.h>
#include "common.h"

unsigned short dma_address;
unsigned short dma_cycle;
unsigned short dma_write_cycle;

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
	if (address == 0x4017) return 0;
	if (address == 0x4016) {
		unsigned char i = get_input();
		printf("  read_memory  %04X -> \033[1;45mJOY\033[0m %04X -> %02X\n", address, address & 0x3FFF, i);
getchar();
		return i;
	}
	fprintf(stdout, "  read_memory  %04X ERR\n", address);
	int x; scanf("%d", &x);
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
		dma_address = data << 8;
		dma_cycle = 0;
		dma_write_cycle = 0;
		cpu_hang();
		return;
	}
	if (address >= 0x4000 && address < 0x4016) {
		printf("  write_memory %04X -> \033[1;45mCTRL\033[0m %04X -> %02X\n", address, address & 0x000F, data);
getchar();
		return;
	}
	if (address == 0x4017) return;
	if (address == 0x4016) {
		reset_input();
		return;
	}

	printf("  write_memory %04X -> %02X ERR\n", address, data);
	exit(3);
}

int memory_auto_transfer(void) {
	printf("memory_auto_transfer %d %04X %d\n", dma_write_cycle, dma_address, dma_cycle);
	if (dma_write_cycle)
		ppu_write(0x2004, RAM[dma_address++]);
	dma_write_cycle = !dma_write_cycle;
	return (++dma_cycle < 512);
}

