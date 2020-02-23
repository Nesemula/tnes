#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "SDL2/SDL.h"
#include "common.h"

static int thread_sdl(void *arg);

SDL_mutex *mutex;
SDL_Thread *thread;
SDL_Window *sdlWindow;
SDL_Renderer *renderer;
SDL_Texture *sdlTexture;

static const Uint8 *key_state;
static Uint32 pix[256 * 240];

static void SetSDLIcon(SDL_Window* window) {
	Uint32 color[6] = { 0x00000000, 0x000000FF, 0xD0A000FF, 0xF8D800FF, 0xA07800FF, 0xF8F8F8FF };
	Uint8 pixel[1024] = {
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1,
		1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 4, 1,
		1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 4, 1,
		1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 4, 4, 1,
		1, 2, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 5, 5, 5, 5, 5, 5, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5, 1, 1, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 5, 5, 5, 5, 5, 5, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5, 1, 1, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 1, 1, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 1, 1, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 1, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 1, 1, 1, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 1, 1, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 1, 1, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 1,
		1, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 1,
		1, 2, 3, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1,
		1, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1,
		1, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1,
		1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	};

	Uint32 icon_data[1024];
	for (int i = 0; i < 1024; i++)
		icon_data[i] = color[pixel[i]];

	unsigned int width = 32;
	unsigned int height = 32;
	unsigned int bytes_per_pixel = 4;
	Uint32 rmask = 0xFF000000, gmask = 0x00FF0000, bmask = 0x0000FF00, amask = 0x000000FF;

	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom((void *) icon_data, width, height, bytes_per_pixel * 8, bytes_per_pixel * width, rmask, gmask, bmask, amask);
	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);

}

void io_setup(void) {
	key_state = SDL_GetKeyboardState(NULL);
	thread = SDL_CreateThread(thread_sdl, "thread_sdl", (void *) NULL);
}

// http://drag.wootest.net/misc/palgen.html
static const Uint32 argb[64] = {
	0xFF464646, 0xFF00065A, 0xFF000678, 0xFF020673, 0xFF35034C, 0xFF57000E, 0xFF5A0000, 0xFF410000,
	0xFF120200, 0xFF001400, 0xFF001E00, 0xFF001E00, 0xFF001521, 0xFF000000, 0xFF000000, 0xFF000000,
	0xFF9D9D9D, 0xFF004AB9, 0xFF0530E1, 0xFF5718DA, 0xFF9F07A7, 0xFFCC0255, 0xFFCF0B00, 0xFFA42300,
	0xFF5C3F00, 0xFF0B5800, 0xFF006600, 0xFF006713, 0xFF005E6E, 0xFF000000, 0xFF000000, 0xFF000000,
	0xFFfeffff, 0xFF1f9eff, 0xFF5376ff, 0xFF9865ff, 0xFFfc67ff, 0xFFff6cb3, 0xFFff7466, 0xFFff8014,
	0xFFc49a00, 0xFF71b300, 0xFF28c421, 0xFF00c874, 0xFF00bfd0, 0xFF2b2b2b, 0xFF000000, 0xFF000000,
	0xFFfeffff, 0xFF9ed5ff, 0xFFafc0ff, 0xFFd0b8ff, 0xFFfebfff, 0xFFffc0e0, 0xFFffc3bd, 0xFFffca9c,
	0xFFe7d58b, 0xFFc5df8e, 0xFFa6e6a3, 0xFF94e8c5, 0xFF92e4eb, 0xFFa7a7a7, 0xFF000000, 0xFF000000
};

int key_count = 0;
Uint8 get_input(void) {
	switch (key_count++) {
		case 0: return key_state[SDL_SCANCODE_KP_5];
		case 1: return key_state[SDL_SCANCODE_KP_4];
		case 2: return key_state[SDL_SCANCODE_KP_6];
		case 3: return key_state[SDL_SCANCODE_RETURN];
		case 4: return key_state[SDL_SCANCODE_E];
		case 5: return key_state[SDL_SCANCODE_D];
		case 6: return key_state[SDL_SCANCODE_S];
		case 7: return key_state[SDL_SCANCODE_F];
	}
	return 0;
}

void reset_input(void) {
	key_count = 0;
}

unsigned char *fframe;
void display_frame(unsigned char *frame) {
	fframe = frame;
	SDL_LockMutex(mutex);
	SDL_Event sdlevent;
	sdlevent.type = SDL_TEXTINPUT;
	SDL_PushEvent(&sdlevent);
}

void sync(void) {
	static Uint32 last_sync = 0;
	SDL_LockMutex(mutex);
#ifdef BENCHMARK
	static Uint32 ticks = 0;
	static Uint32 frames = 0;
	static Uint32 seconds = 1;
	if (!ticks)
		ticks = SDL_GetTicks();
	if (SDL_GetTicks() - ticks >= seconds * 1000) {
		char buffer[24]; 
		sprintf(buffer, "funestus - %dfps", frames / seconds);
		SDL_SetWindowTitle(sdlWindow, buffer);
		seconds++;
	}
	frames++;
#else
	Uint32 passed = (SDL_GetTicks() - last_sync);
	if (passed < 16)
		SDL_Delay(16 - passed);
#endif
	last_sync = SDL_GetTicks();
	SDL_UnlockMutex(mutex);
}

void generate_noise(void) {
	static int band = -4096;
	int len = 256 * 240,
		run = -1,
		color = 0,
		i = 0;

	for (; i < len;) {
		if (run < 0) {
			run = rand() % 6 + 4;
			if (i > band && i < band + 8 * 256) {
				color = rand() % 20;
			}
			else color = rand() % 175;
		}
		run -= 1;
		pix[i] = color | (color << 8) | (color << 16);
		i++;
	}
	band += rand() % (5 * 256);
	if (band > 61440) band = -2048;
}

static int thread_sdl(void *arg) {
	SDL_Init(SDL_INIT_VIDEO);

	sdlWindow = SDL_CreateWindow("funestus", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 768, 720, SDL_WINDOW_SHOWN);
	
	SetSDLIcon(sdlWindow);
	renderer = SDL_CreateRenderer(sdlWindow, -1, 0);
	mutex = SDL_CreateMutex();

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

	SDL_SetRenderTarget(renderer, sdlTexture);

	int go_on = 1;
	while (go_on) {
		SDL_Event event;
		while (SDL_WaitEvent(&event)) {

			if (event.type == SDL_QUIT) {
				go_on = 0;
				break;
			}
			if (event.type == SDL_TEXTINPUT) {
				//generate_noise();
				for (int i = 0; i < 0xF000; i++)
					pix[i] = argb[fframe[i]];
				SDL_UpdateTexture(sdlTexture, NULL, pix, 256 * 4);
				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
				SDL_RenderPresent(renderer);
				SDL_UnlockMutex(mutex);
			}
			if (event.type == SDL_KEYUP)
				if (event.key.keysym.sym == SDLK_o)
					cpu_reset();
		} 
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit(); 

	(void) arg;
	exit(0);
	return 0;
}

