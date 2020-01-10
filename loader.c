#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define prg_size 0x4000
#define chr_size 0x2000
#undef printf

uint8_t *PRG_DATA;
uint8_t *CHR_DATA;
uint8_t prg_banks;
uint8_t chr_banks;
uint8_t mirroring;

static void calculate_crc32(uint8_t *data, size_t length) {
	uint32_t crc = 0xFFFFFFFF;

	for (size_t i = 0; i < length; i++) {
		crc ^= data[i];

		for (int i = 0; i < 8; i++) {
			if (crc & 1)
				crc = ((crc >> 1) ^ 0xEDB88320);
			else
				crc >>= 1;
		}
	}
	printf("CRC32 %08X\n", ~crc);
}

void load_ROM(const char *file_name) {
	FILE *file;
	uint8_t header[16]; // iNES header info
	size_t file_size;
	int invalid = 0; // basic header and file size validation

	file = fopen(file_name, "rb");
	if (!file) {
		puts("File cannot be openned.");
		return;
	}

	fseek(file, 0, SEEK_END); // end of file
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET); // back to beginning of file

	if (!fread(header, 1, sizeof(header), file)) {
		puts("Cannot read header.");
		fclose(file);
		return;
	}
	mirroring = header[6] & 0x01;
	printf("%s mirroring\n", mirroring ? "Vertical" : "Horizontal");

	prg_banks = header[4];
	chr_banks = header[5];
	printf("CHR x %d - PRG x %d\n", chr_banks, prg_banks);

	invalid |= (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A);
	invalid |= (prg_size * prg_banks + chr_size * chr_banks + sizeof(header)) != file_size;
	invalid |= !prg_banks;

	if (invalid) {
		puts("Header information invalid.");
		return;
	}

	PRG_DATA = malloc(prg_size * (prg_banks == 1 ? 2 : prg_banks));
	if (!PRG_DATA || !fread(PRG_DATA, 1, prg_size * prg_banks, file)) {
		puts("Cannot build PRG data.");
		return;
	}

	if (prg_banks == 1)
		memcpy(&PRG_DATA[prg_size], &PRG_DATA[0], prg_size);

	// (chr_banks == 0) means CHR is RAM
	CHR_DATA = malloc(chr_size * (chr_banks ? chr_banks : 1));
	if (!CHR_DATA || (chr_banks && !fread(CHR_DATA, 1, chr_size * chr_banks, file))) {
		puts("Cannot build CHR data.");
		return;
	}

	calculate_crc32(PRG_DATA, prg_size);
	map_program_data(PRG_DATA);
	map_character_data(CHR_DATA);
	fclose(file);
}

