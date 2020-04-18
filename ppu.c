#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

static unsigned int tick;
#define dbg(w, x, y, z) printf("\r" w, x, y, z)

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
			read_buffer = nametable[(ppu_addr & 0x0C00) >> 10][ppu_addr & 0x03FF];
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
		ctrl.nmi_enabled = (data & 0x80);
		ctrl.sprite_double_size = (data & 0x20);
		ctrl.background_pattern_table_index = (data & 0x10);
		ctrl.sprite_pattern_table_index = (data & 0x08);
		ctrl.address_increment = (data & 0x04) ? VERTICAL : HORIZONTAL;
		ctrl.preset_nametable_index = (data & 0x03);
		//if (!status.sprite_zero_hit)
		if (tick >= 81840)
			fprintf(stdout, "ppu_write %d %d\n", ctrl.preset_nametable_index, tick);
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
			nametable[(ppu_addr & 0x0C00) >> 10][ppu_addr & 0x03FF] = data;
			ppu_addr += ctrl.address_increment;
			return;
		}
		if (ppu_addr >= 0x3F00 && ppu_addr < 0x3F20) {
			if (ppu_addr & 0x0003)
				palette[ppu_addr & 0x001F] = data;
			else
				palette[ppu_addr & 0x001F] = palette[ppu_addr & (0x001F ^ 0x0010)] = data;
			ppu_addr += ctrl.address_increment;
			return;
		}
	}
	fprintf(stdout, "    ppu_write %04X %04X %04X %02X\n", ppu_register, ppu_addr, ppu_addr & 0x0FFF, data);
}

static unsigned short _pixel;

#define SPRITE_TOP        (oam[sprite * 4 + 0] + 1)
#define SPRITE_BOTTOM     (oam[sprite * 4 + 0] + 8)
#define TILESET_ENTRY     (oam[sprite * 4 + 1])
#define FLIP_VERTICALLY   (oam[sprite * 4 + 2] & 0x80)
#define FLIP_HORIZONTALLY (oam[sprite * 4 + 2] & 0x40)
#define BEHIND_BACKGROUND (oam[sprite * 4 + 2] & 0x20)
#define PALLETE_COLOR     (oam[sprite * 4 + 2] & 0x03)
#define SPRITE_LEFT       (oam[sprite * 4 + 3] + 0)
#define SPRITE_RIGHT      (oam[sprite * 4 + 3] + 7)

static void draw_pixel(void) {
	//fprintf(stdout, "draw_pixel %d\n", _pixel);
	bool possible_sprite_zero_hit = false;
	bool background_has_priority = true;
	uint_fast8_t pixel_color = 0x00;
	uint_fast8_t default_color = 0x00;

	if (mask.show_sprites) {
		uint8_t x = _pixel & 0xFF, y = _pixel >> 8;
		for (uint_fast8_t sprite = 0; sprite < 64; sprite++) {
			if (SPRITE_TOP > y || SPRITE_BOTTOM < y)
				continue;
			if (SPRITE_LEFT > x || SPRITE_RIGHT < x)
				continue;
			uint8_t vertical_way = FLIP_VERTICALLY ? (SPRITE_BOTTOM - y) : (y - SPRITE_TOP);
			uint16_t pix = (ctrl.sprite_pattern_table_index << 12) | (TILESET_ENTRY << 4) | vertical_way;
			uint8_t pixel_pos = 0x01 << (FLIP_HORIZONTALLY ? (x - SPRITE_LEFT) : (SPRITE_RIGHT - x));

			uint_fast8_t opaque_pixel = (chr[pix] & pixel_pos) ? 0x01 : 0x00;
			opaque_pixel |= (chr[pix | 0x0008] & pixel_pos) ? 0x02 : 0x00;

			if (opaque_pixel) {
				pixel_color = 0x10 | (PALLETE_COLOR << 2) | opaque_pixel;
				background_has_priority = BEHIND_BACKGROUND;
				if (!status.sprite_zero_hit && sprite == 0)
					//possible_sprite_zero_hit = true;
					status.sprite_zero_hit = true;
				break;
			}
		}
	}

	if (background_has_priority) {
		uint_fast8_t attribute = current_nametable[0x03C0 + (((scroll.y & 0xE0) >> 2) | (scroll.x >> 5))];
		uint_fast8_t quadrant = ((scroll.y & 0x10) >> 2) | ((scroll.x & 0x10) >> 3);
		default_color = ((attribute >> quadrant) << 2) & 0x0C;
	}

	if (mask.show_background) {
		if (background_has_priority || possible_sprite_zero_hit) {
			uint_fast8_t chr_index = current_nametable[(scroll.coarse_y << 5) + scroll.coarse_x];
			uint_fast16_t pix = 0x0000 | (ctrl.background_pattern_table_index << 12) | (chr_index << 4) | scroll.fine_y;
			uint_fast8_t opaque_pixel = (chr[pix] & (0x80 >> scroll.fine_x)) ? 0x01 : 0x00;
			opaque_pixel |= (chr[pix | 0x0008] & (0x80 >> scroll.fine_x)) ? 0x02 : 0x00;

			if (opaque_pixel) {
				if (possible_sprite_zero_hit) {
					status.sprite_zero_hit = true;
					//puts("SPRITE_ZERO_HIT_DETECTED");
				}
				if (background_has_priority)
					pixel_color = default_color | opaque_pixel;
			}
		}
	} else {
		pixel_color = default_color;
	}

	frame_buffer[_pixel++] = palette[pixel_color];
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
		///fprintf(stdout, "increment_vertical_scroll %d %d %d\n", ctrl.base_nametable_index, scroll.x, scroll.y);
	}
	current_nametable = nametable[ctrl.base_nametable_index];
	//dbg("prepare_next_frame NT %d | X %03d | Y %03d", ctrl.base_nametable_index, scroll.preset_x, scroll.preset_y);
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
	///fprintf(stdout, "prepare_next_frame %d %d %d\n", ctrl.base_nametable_index, scroll.x, scroll.y);
	//dbg("prepare_next_frame NT %d | X %03d | Y %03d", ctrl.base_nametable_index, scroll.x, scroll.y);
}

void ppu_setup(uint8_t *chr_data, uint8_t chr_banks, uint8_t mirroring) {
	if (chr_banks != 1)
		puts("Unsupported CHR");

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
	current_nametable = nametable[0];
}

void ppu_run(void) {
	//fprintf(stdout, "ppu_run %d\n", tick);
	full_frame[tick++]();
}

