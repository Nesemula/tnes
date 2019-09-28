#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define prg_size 0x4000
#define chr_size 0x2000

unsigned char *PRG_DATA;
unsigned char *CHR_DATA;
int prg_banks;
int chr_banks;

static void calculate_crc32(unsigned char *data, unsigned long length) {
	unsigned long crc = 0xFFFFFFFF;

	for (unsigned int i = 0; i < length; i++) {
		crc ^= data[i];

		for (int i = 0; i < 8; i++) {
			if (crc & 1)
				crc = ((crc >> 1) ^ 0xEDB88320);
			else
				crc >>= 1;
		}
	}
	printf("CRC32 %08lX\n", ~crc & 0xFFFFFFFF);
}

void load_ROM(const char *file_name) {
	FILE *file;
	unsigned char header[16]; // iNES header info
	unsigned long int file_size;
	unsigned int invalid = 0; // basic header and file size validation

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

	prg_banks = header[4];
	chr_banks = header[5];

	invalid |= (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A);
	invalid |= (prg_size * prg_banks + chr_size * chr_banks + sizeof(header)) != file_size;
	invalid |= !prg_banks;

	if (invalid) {
		puts("Header information invalid.");
		return;
	}

	PRG_DATA = malloc(prg_size * prg_banks);
	if (!PRG_DATA || !fread(PRG_DATA, 1, prg_size * prg_banks, file)) {
		puts("Cannot build PRG data.");
		return;
	}

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

