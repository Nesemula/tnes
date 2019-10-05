#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "SDL2/SDL.h"

void* thread_sdl(void *arg);
pthread_t tid;

SDL_Window *sdlWindow;
SDL_Renderer *renderer;
SDL_Texture *sdlTexture;

unsigned char pix[256 * 240 * 4];
unsigned char pixz[512 * 398 * 4];
int band = -4096;

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

void initialize_display(void) {
	pthread_create(&tid, NULL, &thread_sdl, NULL);
}
int z = 0;
void display_frame(unsigned char *frame) {
	for (int i = 0; i < 0xF000; i++) {
		int t = i * 4;
		int color = frame[i] * 85;
		pix[t + 0] = color;
		pix[t + 1] = color;
		pix[t + 2] = color;
		pix[t + 3] = color;
	}
	SDL_Delay(8);
	SDL_Event sdlevent;
	sdlevent.type = SDL_TEXTINPUT;
	SDL_PushEvent(&sdlevent);
}

void upscale(void) {
	unsigned pix_index = 0;
	int scanlines = 0;
	for (int i = 0; i < 398; i++) {
		if (i % 5 == 3 || i % 5 == 4) scanlines = 1;
		else scanlines = 0;
		for (int j = 0; j < 512*4; j++) {
			if (scanlines)
				pixz[i*512*4 + j] = 0x22;
			else {
				pixz[i*512*4 + j] = pix[pix_index];
				if (!(j % 2)) pix_index++;
			}
		}
	}	
}

void generate_noise(void) {
	int len = 61440,
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
		int t = i * 4;
		pix[t] = color;
		pix[t+1] = color;
		pix[t+2] = color;
		pix[t+3] = color;
		i++;
	}
	band += rand() % (5 * 256);
	if (band > 61440) band = -2048;
}

void* thread_sdl(void *arg) {
	SDL_Init(SDL_INIT_VIDEO);

	//sdlWindow = SDL_CreateWindow("TromboNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 398, SDL_WINDOW_SHOWN);
	//sdlWindow = SDL_CreateWindow("TromboNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 240, SDL_WINDOW_SHOWN);
	sdlWindow = SDL_CreateWindow("TromboNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 768, 720, SDL_WINDOW_SHOWN);
	
	SetSDLIcon(sdlWindow);
	renderer = SDL_CreateRenderer(sdlWindow, -1, 0);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	//sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 512, 398);
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
			if (event.type == SDL_TEXTINPUT)
			{
				//generate_noise();

				//upscale();

				//SDL_UpdateTexture(sdlTexture, NULL, pixz, 512 * 4);
				SDL_UpdateTexture(sdlTexture, NULL, pix, 256 * 4);
				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
				SDL_RenderPresent(renderer);
			}
		} 
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit(); 

	(void) arg;
	return NULL;
}

