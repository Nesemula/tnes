#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

enum direction { HORIZONTAL = 1, VERTICAL = 32 };

typedef void render(void);
static render *full_frame[0x015CFE]; // 341x262

static uint8_t *nametable[4];
static uint8_t *current_nametable;
static uint8_t frame_buffer[0xF000]; // 256x240
static uint8_t *chr; // 0x2000
static uint8_t vram[0x800]; // two nametables
static uint8_t oam[0x100];
static uint8_t palette[0x20];

static bool odd_frame;
static bool latch_2nd_write;
static bool rendering_enabled;

// 0x2000 PPUCTRL
static struct {
	bool nmi_enabled;
	bool sprite_double_size;
	bool background_pattern_table_index;
	bool sprite_pattern_table_index;
	uint_fast8_t preset_nametable_index: 2;
	uint_fast8_t base_nametable_index: 2;
	enum direction address_increment;
} ctrl;

// 0x2001 PPUMASK
static struct {
	bool show_sprites;
	bool show_sprites_in_leftmost_8_pixels;
	bool show_background;
	bool show_background_in_leftmost_8_pixels;
} mask;

// 0x2002 PPUSTATUS
static struct {
	bool vblank;
	bool sprite_zero_hit;
	bool sprite_overflow;
} status;

// 0x2003 OAMADDR
static uint8_t oam_addr;

// 0x2005 PPUSCROLL
static struct {
	uint8_t preset_x;
	union {
		uint8_t x;
		struct {
			uint8_t fine_x: 3;
			uint8_t coarse_x: 5;
		};
	};
	uint8_t preset_y;
	union {
		uint8_t y;
		struct {
			uint8_t fine_y: 3;
			uint8_t coarse_y: 5;
		};
	};
} scroll;

// 0x2006 PPUADDR
static uint16_t ppu_addr;

void map_character_data(uint8_t *chr_data, uint8_t mirroring) {
	chr = chr_data;
	nametable[0] = &vram[0x000];
	nametable[3] = &vram[0x400];
	if (mirroring) { // == vertical
		nametable[1] = &vram[0x400];
		nametable[2] = &vram[0x000];
	} else {
		nametable[1] = &vram[0x000];
		nametable[2] = &vram[0x400];
	}
}

uint8_t ppu_read(uint16_t ppu_register) {
	// PPUSTATUS
	if (ppu_register == 0x2002) {
		uint8_t result = (status.vblank ? 0x80 : 0x00) | (status.sprite_zero_hit ? 0x40 : 0x00) | (status.sprite_overflow ? 0x02 : 0x00);
		status.vblank = false;
		latch_2nd_write = false;
		return result;
	}
	// PPUDATA
	if (ppu_register == 0x2007) {
		static uint8_t read_buffer = 0x00;
		uint8_t result = read_buffer;
		if (ppu_addr >= 0x2000 && ppu_addr < 0x3000) {
			if (ppu_addr < 0x2400)
				read_buffer = nametable[0][ppu_addr & 0x03FF];
			else if (ppu_addr < 0x2800)
				read_buffer = nametable[1][ppu_addr & 0x03FF];
			else if (ppu_addr < 0x2C00)
				read_buffer = nametable[2][ppu_addr & 0x03FF];
			else // ppu_addr < 0x3000
				read_buffer = nametable[3][ppu_addr & 0x03FF];
			ppu_addr += ctrl.address_increment;
			return result;
		}
		if (ppu_addr < 0x2000) {
			read_buffer = chr[ppu_addr];
			ppu_addr += ctrl.address_increment;
			return result;
		}
	}
	fprintf(stdout, "    ppu_read %04X %X %04X %04X\n", ppu_register, ppu_register & 0x7, ppu_addr, ppu_addr & 0x3FF);
	return 0x00;
}

void ppu_write(uint16_t ppu_register, uint8_t data) {
	// PPUCTRL
	if (ppu_register == 0x2000) {
		ctrl.nmi_enabled = (data & 0x80) >> 7;
		ctrl.sprite_double_size = (data & 0x20) >> 5;
		ctrl.background_pattern_table_index = (data & 0x10) >> 4;
		ctrl.sprite_pattern_table_index = (data & 0x08) >> 3;
		ctrl.address_increment = (data & 0x04) ? VERTICAL : HORIZONTAL;
		ctrl.preset_nametable_index = (data & 0x03);
		fprintf(stdout, "ppu_write %d\n", ctrl.preset_nametable_index);
		return;
	}
	// PPUMASK
	if (ppu_register == 0x2001) {
		mask.show_sprites = (data & 0x10);
		mask.show_background = (data & 0x08);
		mask.show_sprites_in_leftmost_8_pixels = (data & 0x04);
		mask.show_background_in_leftmost_8_pixels = (data & 0x02);
		rendering_enabled = mask.show_sprites || mask.show_background;
		return;
	}
	// OAMADDR
	if (ppu_register == 0x2003) {
		oam_addr = data;
		return;
	}
	// OAMDATA
	if (ppu_register == 0x2004) {
		oam[oam_addr++] = data;
		return;
	}
	// PPUSCROLL
	if (ppu_register == 0x2005) {
		if (!latch_2nd_write)
			scroll.preset_x = data;
		else
			scroll.preset_y = data;
		latch_2nd_write = !latch_2nd_write;
		return;
	}
	// PPUADDR
	if (ppu_register == 0x2006) {
		if (!latch_2nd_write)
			ppu_addr = data << 8;
		else
			ppu_addr |= data;
		latch_2nd_write = !latch_2nd_write;
		return;
	}
	// PPUDATA
	if (ppu_register == 0x2007) {
		if (ppu_addr >= 0x2000 && ppu_addr < 0x3000) {
			if (ppu_addr < 0x2400)
				nametable[0][ppu_addr & 0x03FF] = data;
			else if (ppu_addr < 0x2800)
				nametable[1][ppu_addr & 0x03FF] = data;
			else if (ppu_addr < 0x2C00)
				nametable[2][ppu_addr & 0x03FF] = data;
			else // ppu_addr < 0x3000
				nametable[3][ppu_addr & 0x03FF] = data;
			ppu_addr += ctrl.address_increment;
			return;
		}
		if (ppu_addr >= 0x3F00 && ppu_addr < 0x3F20) {
			palette[ppu_addr & 0x001F] = data;
			ppu_addr += ctrl.address_increment;
			return;
		}
	}
	fprintf(stdout, "    ppu_write %04X %04X %04X %02X\n", ppu_register, ppu_addr, ppu_addr & 0x0FFF, data);
}

static unsigned short _pixel;
static unsigned int tick;

static void draw_pixel(void) {
	//fprintf(stdout, "draw_pixel %d\n", _pixel);
	unsigned attribute = current_nametable[0x03C0 + (((scroll.y & 0xE0) >> 2) | (scroll.x >> 5))];
	unsigned quadrant = ((scroll.y & 0x10) >> 2) | ((scroll.x & 0x10) >> 3);
	unsigned pal_addr = ((attribute >> quadrant) << 2) & 0x0C;

	if (mask.show_sprites) {
		uint8_t x = _pixel & 0xFF, y = (_pixel >> 8) & 0xFF;
		for (int sprite = 0; sprite < 256; sprite += 4) {
			if (oam[sprite] >= 0xEF) // bellow the screen
				continue;
			if ((oam[sprite] + 0) > y || (oam[sprite] + 7) < y)
				continue;
			if ((oam[sprite + 3]) > x || (oam[sprite + 3] + 7) < x)
				continue;
			int oam_table_entry = oam[sprite + 1];
			uint16_t pix = 0x0000 | (ctrl.sprite_pattern_table_index << 12) | (oam_table_entry << 4) | (y - oam[sprite]);
			uint8_t pixel_pos = (oam[sprite + 2] & 0x40) ? (0x01 << (x - oam[sprite + 3])) : (0x80 >> (x - oam[sprite + 3]));

			int a = chr[pix] & pixel_pos;
			int b = chr[pix | 0x0008] & pixel_pos;
			pal_addr += (a ? 1 : 0) + (b ? 2 : 0);
			if ((a || b) && ! sprite) status.sprite_zero_hit = true;
			if ((a || b) && ! sprite) fprintf(stdout, "zero %d\n", _pixel);
			break;
		}
	}
	if (mask.show_background && !(pal_addr & 0x03)) {
		uint16_t chr_index = current_nametable[(scroll.coarse_y << 5) + scroll.coarse_x];
		uint16_t pix = 0x0000 | (ctrl.background_pattern_table_index << 12) | (chr_index << 4) | scroll.fine_y;
		int a = chr[pix] & (0x80 >> scroll.fine_x);
		int b = chr[pix | 0x0008] & (0x80 >> scroll.fine_x);
		pal_addr += (a ? 1 : 0) + (b ? 2 : 0);
	}

	frame_buffer[_pixel++] = palette[pal_addr];
	if (!++scroll.x)
		current_nametable = nametable[ctrl.base_nametable_index ^ 0x1];
}

static void increment_vertical_scroll(void) {
	//fprintf(stdout, "increment_vertical_scroll %d %d\n", scroll.coarse_y, scroll.fine_y);
	scroll.x = scroll.preset_x;
	ctrl.base_nametable_index = (ctrl.base_nametable_index & 0x2) | (ctrl.preset_nametable_index & 0x1);
	if (++scroll.y == 240) {
		scroll.y = 0;
		ctrl.base_nametable_index ^= 0x2;
		fprintf(stdout, "increment_vertical_scroll %d %d %d\n", ctrl.base_nametable_index, scroll.x, scroll.y);
	}
	current_nametable = nametable[ctrl.base_nametable_index];
}

static void idle(void) {
	//fprintf(stdout, "idle %d\n", _pixel);
	return;
}

static void set_vblank_flag(void) {
	//fprintf(stdout, "set_vblank_flag %d\n", _pixel);
	status.vblank = true;
	if (ctrl.nmi_enabled)
		cpu_interrupt();
}

static void unset_vblank_flag(void) {
	//fprintf(stdout, "unset_vblank_flag %d\n", _pixel);
	status.vblank = false;
}

static void prepare_next_frame(void) {
	//fprintf(stdout, "prepare_next_frame %d\n", _pixel);
	tick = (rendering_enabled && odd_frame) ? 1 : 0;
	odd_frame = !odd_frame;
	_pixel = 0;
	status.sprite_zero_hit = false;
	ctrl.base_nametable_index = ctrl.preset_nametable_index;
	current_nametable = nametable[ctrl.base_nametable_index];
	scroll.x = scroll.preset_x;
	scroll.y = scroll.preset_y;
	display_frame(frame_buffer);
	sync();
	fprintf(stdout, "prepare_next_frame %d %d %d\n", ctrl.base_nametable_index, scroll.x, scroll.y);
}

void ppu_setup(void) {
	for (register uint_fast32_t i = 0; i < 89342; i++) {
		if (i < 81840) {
			if ((i % 341 < 257) && (i % 341))
				full_frame[i] = draw_pixel;
			else if (i % 341 == 340)
				full_frame[i] = increment_vertical_scroll;
			else
				full_frame[i] = idle;
		} else {
			full_frame[i] = idle;
		}
	}
	full_frame[82182] = set_vblank_flag;
	full_frame[89002] = unset_vblank_flag;
	full_frame[89341] = prepare_next_frame;
	current_nametable = nametable[0];
}

void ppu_run(void) {
	//fprintf(stdout, "ppu_run %d\n", tick);
	full_frame[tick++]();
}

