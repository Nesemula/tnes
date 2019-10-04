#include <stdio.h>
#include <stdlib.h>
#include "common.h"

unsigned char FRAME_BUFFER[0xF000];
unsigned char *CHR; // 0x2000
unsigned char VRAM[0x800];
unsigned char OAM[0xFF];

char is_odd_frame = 0;
char is_rendering = 0;

// 0x2000 PPUCTRL
char NMI_enabled = 0;
char background_pattern_table_address = 0;
char sprite_pattern_table_address = 0;
char VRAM_address_increment = 1; // per CPU read/write of PPUDATA
char base_nametable_address = 0;

// 0x2001 PPUMASK
char show_sprites = 0;
char show_sprites_in_leftmost_8_pixels = 0;
char show_background = 0;
char show_background_in_leftmost_8_pixels = 0;

// 0x2002 PPUSTATUS
char vblank = 0;
char sprite_zero_hit = 0;
char sprite_overflow = 0;

// 0x2003 OAMADDR
unsigned short OAM_address = 0;

// 0x2005 PPUSCROLL
unsigned char scroll_write = 0;
unsigned char scroll_x = 0;
unsigned char scroll_y = 0;

// 0x2006 PPUADDR
unsigned short ppu_addr = 0;
unsigned char address_write = 0;

// 0x2007 PPUADDR

void map_character_data(unsigned char *chr_data) {
	CHR = chr_data;
}

unsigned char ppu_read(unsigned short ppu_register) {
	if (ppu_register == 0x2002) {
		unsigned char result = (vblank ? 0x80 : 0x00) | (sprite_zero_hit ? 0x40 : 0x00) | (sprite_overflow ? 0x02 : 0x00);
		vblank = 0;
		scroll_write = 0;
		address_write = 0;
		printf("    ppu_read  %04X -> PPUSTATUS %02X\n", ppu_register, result);
		return result;
	}
	if (ppu_register == 0x2007) {
		//mario bros
		return 0;
	}
#undef printf
	printf("    ppu_read  %04X ERR\n", ppu_register);
#define printf 0&&printf
	exit(4);
}

void ppu_write(unsigned short ppu_register, unsigned char data) {
	if (ppu_register == 0x2000) {
		printf("    ppu_write %04X -> PPUCTRL   %02X\n", ppu_register, data);
		NMI_enabled = (data & 0x80) >> 7;
		background_pattern_table_address = (data & 0x10) >> 4;
		sprite_pattern_table_address = (data & 0x08) >> 3;
		VRAM_address_increment = ((data & 0x04) >> 2) ? 32 : 1;
		base_nametable_address = (data & 0x03);
		printf("      NMI_enabled %d\n", NMI_enabled);
		printf("      background_pattern_table_address %d\n", background_pattern_table_address);
		printf("      sprite_pattern_table_address %d\n", sprite_pattern_table_address);
		printf("      VRAM_address_increment %d\n", VRAM_address_increment);
		printf("      base_nametable_address %d\n", base_nametable_address);
getchar();
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
		OAM_address = data;
getchar();
		return;
	}
	if (ppu_register == 0x04) {
		printf("    ppu_write %02X -> OAMDATA   %02X\n", ppu_register, data);
		OAM[OAM_address++] = data;
		printf("      OAM_address[%02X] -> %02X\n", OAM_address - 1, data);
getchar();
		return;
	}
	if (ppu_register == 0x2005) {
		printf("    ppu_write %04X -> PPUSCROLL %02X\n", ppu_register, data);
		printf("      scroll_write %d\n", ((scroll_write + 1) % 2));
		if ((scroll_write = ((scroll_write + 1) % 2)))
			scroll_x = data;
		else
			scroll_y = data;
		printf("      scroll_x %d\n", scroll_x);
		printf("      scroll_y %d\n", scroll_y);
getchar();
		return;
	}
	if (ppu_register == 0x2006) {
		printf("    ppu_write %04X -> PPUADDR   %02X\n", ppu_register, data);
		printf("      address_write %d\n", ((address_write + 1) % 2));
		printf("      ppu_addr %04X\n", ppu_addr);
		if ((address_write = ((address_write + 1) % 2)))
			ppu_addr = data << 8;
		else
			ppu_addr |= data;
		printf("      ppu_addr %04X\n", ppu_addr);
getchar();
		return;
	}
	if (ppu_register == 0x2007) {
		printf("    ppu_write %04X -> PPUDATA   %02X\n", ppu_register, data);
		VRAM[ppu_addr & 0x07FF] = data;
		printf("      ppu_addr %04X -> %02X\n", ppu_addr & 0x07FF, data);
		ppu_addr += VRAM_address_increment;
		return;
	}
//	if (ppu_register == 0x14) {
//		printf("    ppu_write %02X -> OAMDMA    %02X\n", ppu_register, data);
//		cpu_dma(data);
//		return;
//	}
	printf("    ppu_write %02X ERR %02X\n", ppu_register, data);
	exit(5);
}

void generate_buffer(unsigned char scanline) {
	int buffer_entry = scanline * 256;
	int pixel_line = scanline % 8;
	for (int tile = 0; tile < 32; tile++) {
		int name_table_entry = tile + ((scanline / 8) * 32);
		int pattern_table_entry = VRAM[name_table_entry] * 16 + pixel_line + (background_pattern_table_address ? 0x1000 : 0);
		for (int pixel = 0; pixel < 8; pixel++) {
			int a = CHR[pattern_table_entry] & (0x80 >> pixel);
			int b = CHR[pattern_table_entry + 8] & (0x80 >> pixel);
			FRAME_BUFFER[buffer_entry++] = (a ? 1 : 0) + (b ? 2 : 0);
		}
		name_table_entry += 16;
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
	}
	if (pixel == 340 && is_odd_frame && is_rendering) {
		is_odd_frame = 0;
		scanline = pixel = 0;
	}
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

