#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"

#define PRG_BANK_SIZE 0x4000
#define CHR_BANK_SIZE 0x2000
#define HEADER_OFFSET 0x10

static uint32_t calculate_crc32(uint8_t *data, size_t length) {
	uint32_t crc = 0xFFFFFFFF;

	for (size_t i = 0; i < length; i++) {
		crc ^= data[i];

		for (uint_fast8_t i = 0; i < 8; i++) {
			if (crc & 1)
				crc = ((crc >> 1) ^ 0xEDB88320);
			else
				crc >>= 1;
		}
	}
	return ~crc;
}

int main(int argc, char *argv[]) {
	puts("NES emulator under development");

	if (argc < 2) {
		puts("Missing ROM filename");
		return EXIT_FAILURE;
	}

	FILE *rom_file = fopen(argv[1], "rb");
	if (rom_file == NULL) {
		perror("Error opening ROM");
		return EXIT_FAILURE;
	}

	fseek(rom_file, 0, SEEK_END);
	long rom_size = ftell(rom_file);
	rewind(rom_file);

	uint8_t *rom_data = malloc(rom_size);
	if (rom_data == NULL) {
		perror("Error allocating memory");
		return EXIT_FAILURE;
	}

	size_t read_result = fread(rom_data, 1, rom_size, rom_file);
	if (read_result - rom_size != 0) {
		puts("Error reading ROM");
		return EXIT_FAILURE;
	}
	fclose(rom_file);

	// iNES header info
	uint8_t prg_banks = rom_data[4];
	uint8_t chr_banks = rom_data[5];
	uint8_t mirroring = rom_data[6] & 0x01;
	uint8_t mapper = (rom_data[6] >> 4) | (rom_data[7] & 0xF0);

	// basic header and file size validation
	if (rom_data[0] != 'N' || rom_data[1] != 'E' || rom_data[2] != 'S' || rom_data[3] != 0x1A) {
		puts("Invalid ROM header");
		return EXIT_FAILURE;
	}

	if ((PRG_BANK_SIZE * prg_banks + CHR_BANK_SIZE * chr_banks + HEADER_OFFSET) != rom_size || !prg_banks) {
		puts("Invalid ROM size");
		return EXIT_FAILURE;
	}

	// when chr_banks is zero, it means CHR is RAM
	uint8_t *chr_ram;
	if (chr_banks == 0) {
		chr_ram = malloc(CHR_BANK_SIZE);
		if (chr_ram == NULL) {
			perror("Error allocating memory");
			return EXIT_FAILURE;
		}
	}

	printf("CHR 8k banks x%d\n", chr_banks);
	printf("PRG 16k banks x%d\n", prg_banks);
	printf("CHR mirroring -> %s\n", mirroring ? "vertical" : "horizontal");
	printf("iNES mapper -> %d\n", mapper);
	printf("CRC32 -> %08X\n", calculate_crc32(&rom_data[HEADER_OFFSET], rom_size - HEADER_OFFSET));

	cpu_setup(&rom_data[HEADER_OFFSET], prg_banks);
	ppu_setup(chr_banks ? &rom_data[prg_banks * PRG_BANK_SIZE + HEADER_OFFSET] : chr_ram, chr_banks, mirroring);
	io_setup();

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

