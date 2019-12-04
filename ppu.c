#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "common.h"

extern unsigned char mirroring;

unsigned char *nametable_1;
unsigned char *nametable_2;
unsigned char *nametable_3;
unsigned char *nametable_4;

unsigned char *nametable_base;
unsigned char *nametable_vert;
unsigned char *nametable_horz;
unsigned char *nametable_diag;

unsigned char FRAME_BUFFER[0xF000];
unsigned char *CHR; // 0x2000
unsigned char VRAM[0x800];
unsigned char OAM[0x100];
unsigned char paletteRAM[0x20];

char is_odd_frame = 0;
char is_rendering = 0;

// 0x2000 PPUCTRL
char NMI_enabled = 0;
char sprite_size = 0;
char background_pattern_table_address = 0;
char sprite_pattern_table_address = 0;
char VRAM_address_increment = 1; // per CPU read/write of PPUDATA
char base_nametable_address = 0;

// 0x2001 PPUMASK
char show_sprites = 0;
char show_sprites_in_leftmost_8_pixels = 0;
char show_background = 0;
char show_background_in_leftmost_8_pixels = 0;

// 0x2004 OAMDATA

// 0x2002 PPUSTATUS
char vblank = 0;
char sprite_zero_hit = 0;
char sprite_overflow = 0;

// 0x2003 OAMADDR
unsigned char obj_addr = 0;

// 0x2005 PPUSCROLL
bool last_scroll_write = false;
unsigned char scroll_x = 0;
unsigned char scroll_y = 0;

// 0x2006 PPUADDR
bool last_addr_write = false;
unsigned short ppu_addr = 0;

// 0x2007 PPUADDR

void map_character_data(unsigned char *chr_data) {
	CHR = chr_data;
	if (mirroring) { // == vertical
		nametable_1 = &VRAM[0x000];
		nametable_2 = &VRAM[0x400];
		nametable_3 = &VRAM[0x000];
		nametable_4 = &VRAM[0x400];
	} else {
		nametable_1 = &VRAM[0x000];
		nametable_2 = &VRAM[0x000];
		nametable_3 = &VRAM[0x400];
		nametable_4 = &VRAM[0x400];
	}
	nametable_base = nametable_1;
	nametable_horz = nametable_2;
	nametable_vert = nametable_3;
	nametable_diag = nametable_4;
}

unsigned char ppu_read(unsigned short ppu_register) {
	if (ppu_register == 0x2002) {
		unsigned char result = (vblank ? 0x80 : 0x00) | (sprite_zero_hit ? 0x40 : 0x00) | (sprite_overflow ? 0x02 : 0x00);
		vblank = 0;
		last_scroll_write = false;
		last_addr_write = false;
		printf("    ppu_read  %04X -> PPUSTATUS %02X\n", ppu_register, result);
		return result;
	}
	if (ppu_register == 0x2007) {
		static unsigned char data = 0;
		unsigned char read_buffer = data;
		printf("      ppu_addr %04X -> %02X\n", ppu_addr, data);
		//if (ppu_addr >= 0x3F00 && ppu_addr < 0x4000) {
		if (0) {
			read_buffer = data = paletteRAM[ppu_addr & 0x1F];
			ppu_addr += VRAM_address_increment;
			return read_buffer;
		}
		if (ppu_addr >= 0x2000 && ppu_addr < 0x3F00) {
			unsigned short real_addr = ppu_addr & 0x3FF;
			unsigned short full_addr = ppu_addr & 0xFFF;
			if (full_addr < 0x400)
				data = nametable_1[real_addr];
			else if (full_addr < 0x800)
				data = nametable_2[real_addr];
			else if (full_addr < 0xC00)
				data = nametable_3[real_addr];
			else // full_addr < 0x1000
				data = nametable_4[real_addr];
			ppu_addr += VRAM_address_increment;
			return read_buffer;
		}
		if (ppu_addr < 0x2000) {
			data = CHR[ppu_addr];
			ppu_addr += VRAM_address_increment;
			return read_buffer;
		}
	}
	fprintf(stdout, "    ppu_read %04X %04X %04X\n", ppu_register, ppu_addr, ppu_addr & 0xFFF);
	exit(4);
}

void ppu_write(unsigned short ppu_register, unsigned char data) {
	if (ppu_register == 0x2000) {
		printf("    ppu_write %04X -> PPUCTRL   %02X\n", ppu_register, data);
		NMI_enabled = (data & 0x80) >> 7;
		sprite_size = (data & 0x20) >> 5;
		background_pattern_table_address = (data & 0x10) >> 4;
		sprite_pattern_table_address = (data & 0x08) >> 3;
		VRAM_address_increment = (data & 0x04) ? 32 : 1;
		base_nametable_address = (data & 0x03);
		printf("      NMI_enabled %d\n", NMI_enabled);
		printf("      sprite_size %d\n", sprite_size);
		printf("      background_pattern_table_address %d\n", background_pattern_table_address);
		printf("      sprite_pattern_table_address %d\n", sprite_pattern_table_address);
		printf("      VRAM_address_increment %d\n", VRAM_address_increment);
		printf("      base_nametable_address %d\n", base_nametable_address);
		if (sprite_size) fprintf(stdout,"      sprite_size %d\n", sprite_size);
		switch (data & 0x03) {
			case 0:
				nametable_base = nametable_1;
				nametable_horz = nametable_2;
				nametable_vert = nametable_3;
				nametable_diag = nametable_4;
				break;
			case 1:
				nametable_base = nametable_2;
				nametable_horz = nametable_1;
				nametable_vert = nametable_4;
				nametable_diag = nametable_3;
				break;
			case 2:
				nametable_base = nametable_3;
				nametable_horz = nametable_4;
				nametable_vert = nametable_1;
				nametable_diag = nametable_2;
				break;
			case 3:
				nametable_base = nametable_4;
				nametable_horz = nametable_3;
				nametable_vert = nametable_2;
				nametable_diag = nametable_1;
				break;
		}
		return;
	}
	if (ppu_register == 0x2001) {
		printf("    ppu_write %04X -> PPUMASK   %02X\n", ppu_register, data);
		show_sprites = (data & 0x10) >> 4;
		show_background = (data & 0x08) >> 3;
		show_sprites_in_leftmost_8_pixels = (data & 0x04) >> 2;
		show_background_in_leftmost_8_pixels = (data & 0x02) >> 1;
		is_rendering = show_sprites || show_background;
		printf("      show_sprites %d\n", show_sprites);
		printf("      show_background %d\n", show_background);
		printf("      show_sprites_in_leftmost_8_pixels %d\n", show_sprites_in_leftmost_8_pixels);
		printf("      show_background_in_leftmost_8_pixels %d\n", show_background_in_leftmost_8_pixels);
		printf("      is_rendering %d\n", is_rendering);
getchar();
		return;
	}
	if (ppu_register == 0x2003) {
		printf("    ppu_write %04X -> OAMADDR   %02X\n", ppu_register, data);
		obj_addr = data;
getchar();
		return;
	}
	if (ppu_register == 0x2004) {
		printf("    ppu_write %04X -> OAMDATA   %02X\n", ppu_register, data);
		printf("      obj_addr[%02X] -> %02X\n", obj_addr, data);
		OAM[obj_addr++] = data;
		return;
	}
	if (ppu_register == 0x2005) {
		printf("    ppu_write %04X -> PPUSCROLL %02X\n", ppu_register, data);
		last_scroll_write = !last_scroll_write;
		printf("      last_scroll_write %d\n", last_scroll_write);
		if (last_scroll_write)
			scroll_x = data;
		else
			scroll_y = data;
		return;
	}
	if (ppu_register == 0x2006) {
		printf("    ppu_write %04X -> PPUADDR   %02X\n", ppu_register, data);
		last_addr_write = !last_addr_write;
		printf("      last_addr_write %d\n", last_addr_write);
		printf("      ppu_addr %04X\n", ppu_addr);
		if (last_addr_write)
			ppu_addr = data << 8;
		else
			ppu_addr |= data;
		printf("      ppu_addr %04X\n", ppu_addr);
		return;
	}
	if (ppu_register == 0x2007) {
		printf("    ppu_write %04X -> PPUDATA   %02X\n", ppu_register, data);
		if (ppu_addr >= 0x3F00 && ppu_addr < 0x4000) {
			unsigned char low_nibble = ppu_addr & 0xF;
			if (!(low_nibble % 4)) // addresses 0x3F00, 0x3F04, 0x3F08, 0x3F0C, 0x3100, 0x3F14, 0x3F18, 0x3F1C
				paletteRAM[low_nibble] = paletteRAM[low_nibble + 0x10] = data;
			else
				paletteRAM[ppu_addr & 0x1F] = data;
			ppu_addr += VRAM_address_increment;
			return;
		}
		if (ppu_addr >= 0x2000 && ppu_addr < 0x3F00) {
			unsigned short real_addr = ppu_addr & 0x3FF;
			unsigned short full_addr = ppu_addr & 0xFFF;
			if (full_addr < 0x400)
				nametable_1[real_addr] = data;
			else if (full_addr < 0x800)
				nametable_2[real_addr] = data;
			else if (full_addr < 0xC00)
				nametable_3[real_addr] = data;
			else // full_addr < 0x1000
				nametable_4[real_addr] = data;
			ppu_addr += VRAM_address_increment;
			return;
		}
	}
	fprintf(stdout, "    ppu_write %04X %04X %04X %02X\n", ppu_register, ppu_addr, ppu_addr & 0xFFF, data);
	exit(5);
}

void generate_buffer(unsigned char scanline) {
	unsigned char pscan = scanline;
	int buffer_entry = scanline * 256;
	unsigned char *nametable_baseZ, *nametable_horzW;
	if (scanline + scroll_y < 240) {
		scanline = (scanline + scroll_y);
		nametable_baseZ = nametable_base;
		nametable_horzW = nametable_horz;
	} else {
		scanline = ((scanline + scroll_y) % 240);
		nametable_baseZ = nametable_vert;
		nametable_horzW = nametable_diag;
	}

	int pixel_line = scanline % 8;
	int start = scroll_x / 8; // start column
	int offset = scroll_x % 8; // offset in first and last tile
	//for (int tile = 0; tile < 32; tile++) {
	for (int tile = start; tile < 32; tile++) {
		int attribute = nametable_baseZ[0x03C0 + (scanline / 32 * 8) + tile / 4];
		int line = (scanline / 16);
		int col = tile / 2;
		int pallete;
		//printf("%3d %2d %02X %02X %2d %2d\n", scanline, tile, 0x03C0 + (scanline / 32 * 8) + tile / 4, attribute, col, line);
		if (line & 1 && col & 1)
			pallete = (attribute & 0xC0) >> 6;
		else if (line & 1)
			pallete = (attribute & 0x30) >> 4;
		else if (col & 1)
			pallete = (attribute & 0x0C) >> 2;
		else
			pallete = (attribute & 0x03) >> 0;
		int pal_addr = pallete * 4;

		int name_table_entry = tile + ((scanline / 8) * 32);
		int pattern_table_entry = nametable_baseZ[name_table_entry] * 16 + pixel_line + (background_pattern_table_address ? 0x1000 : 0);
		//for (int pixel = 0; pixel < 8; pixel++) {
		int p_init = (tile == start) ? offset : 0;
		for (int pixel = p_init; pixel < 8; pixel++) {
			int a = CHR[pattern_table_entry] & (0x80 >> pixel);
			int b = CHR[pattern_table_entry + 8] & (0x80 >> pixel);
			//FRAME_BUFFER[buffer_entry++] = (a ? 1 : 0) + (b ? 2 : 0);
			FRAME_BUFFER[buffer_entry++] = paletteRAM[show_background ? pal_addr + (a ? 1 : 0) + (b ? 2 : 0) : pal_addr];
		}
		name_table_entry += 16;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int tile = 0; tile <= start; tile++) {
		int attribute = nametable_horzW[0x03C0 + (scanline / 32 * 8) + tile / 4];
		int line = (scanline / 16);
		int col = tile / 2;
		int pallete;
		if (line & 1 && col & 1)
			pallete = (attribute & 0xC0) >> 6;
		else if (line & 1)
			pallete = (attribute & 0x30) >> 4;
		else if (col & 1)
			pallete = (attribute & 0x0C) >> 2;
		else
			pallete = (attribute & 0x03) >> 0;
		int pal_addr = pallete * 4;

		int name_table_entry = tile + ((scanline / 8) * 32);
		int pattern_table_entry = nametable_horzW[name_table_entry] * 16 + pixel_line + (background_pattern_table_address ? 0x1000 : 0);
		int p_end = (tile == start) ? offset : 8;
		for (int pixel = 0; pixel < p_end; pixel++) {
			int a = CHR[pattern_table_entry] & (0x80 >> pixel);
			int b = CHR[pattern_table_entry + 8] & (0x80 >> pixel);
			FRAME_BUFFER[buffer_entry++] = paletteRAM[show_background ? pal_addr + (a ? 1 : 0) + (b ? 2 : 0) : pal_addr];
		}
		name_table_entry += 16;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////
	buffer_entry = pscan * 256;
	if (show_sprites)
		for (int sprite = 252; sprite >= 0; sprite -= 4) {
			if (OAM[sprite] >= 0xEF)
				continue;
			if ((OAM[sprite]+1) > pscan || (OAM[sprite]+1) < pscan - 7)
				continue;
			int oam_table_entry = OAM[sprite + 1];
			int oam_line = (OAM[sprite + 2] & 0x80) ? (OAM[sprite]+8 - pscan) : (pscan - OAM[sprite]-1); // vertical mirror
			int pattern_table_entry = oam_table_entry * 16 + oam_line + (sprite_pattern_table_address ? 0x1000 : 0);
			int pal_addr = 0x10 + (OAM[sprite + 2] & 0x03) * 4;
			if (OAM[sprite + 2] & 0x40) // horizontal mirrror
				for (int pixel = 0; pixel < 8; pixel++) {
					int a = CHR[pattern_table_entry] & (0x01 << pixel);
					int b = CHR[pattern_table_entry + 8] & (0x01 << pixel);
					if (a + b)
						if (!((OAM[sprite + 2] & 0x20) && FRAME_BUFFER[buffer_entry + OAM[sprite + 3] + pixel] != paletteRAM[0]))
							FRAME_BUFFER[buffer_entry + OAM[sprite + 3] + pixel] = paletteRAM[pal_addr + (a ? 1 : 0) + (b ? 2 : 0)];
					if (a + b && FRAME_BUFFER[buffer_entry + OAM[sprite + 3] + pixel] != paletteRAM[0])
						sprite_zero_hit = 1;
				}
			else // no horizontal mirror
				for (int pixel = 0; pixel < 8; pixel++) {
					int a = CHR[pattern_table_entry] & (0x80 >> pixel);
					int b = CHR[pattern_table_entry + 8] & (0x80 >> pixel);
					if (a + b)
						if (!((OAM[sprite + 2] & 0x20) && FRAME_BUFFER[buffer_entry + OAM[sprite + 3] + pixel] != paletteRAM[0]))
							FRAME_BUFFER[buffer_entry + OAM[sprite + 3] + pixel] = paletteRAM[pal_addr + (a ? 1 : 0) + (b ? 2 : 0)];
					if (a + b && FRAME_BUFFER[buffer_entry + OAM[sprite + 3] + pixel] != paletteRAM[0])
						sprite_zero_hit = 1;
				}

		}
}

void generate_buffer_pattern_table(unsigned char scanline) {
	if (scanline > 127) return;
	int buffer_entry = scanline * 256;
	int pattern_table_tile = (scanline / 8) * 512;
	int pixel_line = scanline % 8;
	for (int tile = 0; tile < 32; tile++) {
		int pattern_table_entry = pattern_table_tile + pixel_line;
		for (int pixel = 0; pixel < 8; pixel++) {
			int a = CHR[pattern_table_entry] & (0x80 >> pixel);
			int b = CHR[pattern_table_entry + 8] & (0x80 >> pixel);
			FRAME_BUFFER[buffer_entry++] = (a ? 1 : 0) + (b ? 2 : 0);
		}
		pattern_table_tile += 16;
	}
}

int pixel = 0;
int scanline = 0;

void run_visible_scanline(void) {
	//printf("visible %3d %3d\n", scanline, pixel);
	if (pixel == 255)
		generate_buffer(scanline);
		//generate_buffer_pattern_table(scanline);
	if (++pixel > 340)
		pixel = 0, scanline++;
}

void run_post_render_scanline(void) {
	//printf("post %3d %3d\n", scanline, pixel);
	if (pixel == 0)
		display_frame(FRAME_BUFFER);
	if (pixel++ == 340)
		scanline++, pixel = 0;
}

void run_vblank(void) {
	//printf("vblank %3d %3d\n", scanline, pixel);
	if (scanline == 241 && pixel == 1) {
		if (NMI_enabled)
			cpu_interrupt();
		vblank = 1;
	}
	if (pixel++ == 340)
		pixel = 0, scanline++;
}

void run_pre_render_scanline(void) {
	//printf("pre %3d %3d\n", scanline, pixel);
	if (pixel == 1)
		vblank = 0;
	if (pixel++ == 340) {
		is_odd_frame = !is_odd_frame;
		scanline = pixel = 0;
		sync();
	}
	if (pixel == 340 && is_odd_frame && is_rendering) {
		is_odd_frame = 0;
		scanline = pixel = 0;
		sync();
	}
	sprite_zero_hit = 0;
}

void ppu_exec(void) {
	if (scanline < 240)
		run_visible_scanline();
	else if (scanline == 240)
		run_post_render_scanline();
	else if (scanline < 261)
		run_vblank();
	else // scanline == 261
		run_pre_render_scanline();
}

void force_display(void) {
	for (int i = 0; i < 240; i++)
		generate_buffer(i);
	display_frame(FRAME_BUFFER);
}

