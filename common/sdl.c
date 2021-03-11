#include <stdio.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <getopt.h>
#include "u2gl.h"

static volatile uint32_t tick_timer = 0;

SDL_Window *win = 0;
SDL_GLContext glcontext;

#define MAX_CONFIGS 10

char *argv0;
int window_width = 640;
int window_height = 480;
static int frame_dump = 0;

static void dump_frame(void);

Uint32 timer_function(Uint32 interval, void *param)
{
	tick_timer++;
	return interval;
}

static void close_window()
{
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(win);
}

extern char *optarg;

#define OPTIONS "ds:"
static int parse_options(int argc, char **argv)
{
	int o;

	while ((o = getopt(argc, argv, OPTIONS)) != -1) {
		switch (o) {
		case 'd':
			frame_dump = 1;
			break;
		case 's':
			if ((strtok(optarg, "x"))) {
				window_width = atoi(optarg);
				window_height = atoi(strtok(NULL, ""));
				printf("window size: %dx%d\n",
						window_width, window_height);
			}
			break;
		default:
			return -1;
		}
	}

	return 0;
}

int init_graphics(char *caption, int argc, char **argv)
{
	SDL_SysWMinfo sysInfo;

	argv0 = argv[0];

	if (parse_options(argc, argv) < 0)
		return -1;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "error: %s\n", SDL_GetError());
		return -1;
	}

	atexit(SDL_Quit);

	win = SDL_CreateWindow(caption, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (win == NULL) {
		fprintf(stderr, "error: SDL_CreateWindow failed\n");
		return -1;
	}

	/* Get window manager information */
	SDL_VERSION(&sysInfo.version);
	if (SDL_GetWindowWMInfo(win, &sysInfo) == SDL_FALSE) {
		close_window();
		fprintf(stderr, "error: SDL_GetWindowWMInfo failed\n");
		return -1;
	}

	glcontext = SDL_GL_CreateContext(win);

	if (glcontext == NULL) {
		close_window();
		fprintf(stderr, "error: SDL_GL_CreateContext failed\n");
		return -1;
	}

	if (SDL_GL_MakeCurrent(win, glcontext) < 0) {
		close_window();
		fprintf(stderr, "error: SDL_GL_MakeCurrent failed\n");
		return -1;
	}

	tick_timer = 0;
	SDL_AddTimer(4, timer_function, 0);

	return 0;
}

static long frameus = 1000000 / 70;
static long deltaus;

void set_fps(float fps)
{
	frameus = 1000000 / fps;
}

void swap_buffers()
{
	static long frames, startus, oldus = -1;
	struct timeval tv;
	long us;
	float fps, avg;

	gettimeofday(&tv, NULL);
	us = tv.tv_sec * 1000000 + tv.tv_usec;

	if (oldus < 0) {
		startus = oldus = us;
		frames = 0;
	}

	deltaus = us - oldus;
	if (frame_dump) {
		deltaus = 1000000 / 60;
	}
	oldus = us;

	if (deltaus > 0) {
		fps = 1000000.0 / deltaus;
		avg = 1000.0 * frames / ((us - startus) / 1000.0);
	} else {
		avg = fps = 0.0;
	}

	frames++;

	printf("fps = %5.1f, avg = %5.1f    \r", fps, avg);
	
	if (frame_dump)
		dump_frame();
	SDL_GL_SwapWindow(win);
}

int adjust_framerate()
{
	static long adjust = 0;
	int num_frames;

	adjust += deltaus;

	num_frames = adjust / frameus;
	//printf("deltaus=%ld  frameus=%ld  adjust=%ld  num_frames=%d\n", deltaus, frameus, adjust, num_frames);

	adjust -= frameus * num_frames;

	return num_frames;
}

#if 0
static void timer_delay()
{
	static uint32_t m = 0;

	if (tick_timer < m)
		m = 0;

	while (tick_timer - m < 2) {
		SDL_Delay(1);
	}
	m = tick_timer;
}
#endif

void poll_event()
{
	SDL_Event event;

	//timer_delay();

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			break;
		case SDL_QUIT:
			/* SDL_Quit(); */
			exit(0);
			break;
		}
	}
}

static void dump_frame()
{
	static char *buffer = NULL;
	static int num;
	char name[80], *src, *dest;
	FILE *f;
	int i;

	if (buffer == NULL) {
		//glReadBuffer(GL_BACK);
		buffer = malloc(4 * window_width * window_height);
		num = 0;
	}

	glReadPixels(0, 0, window_width, window_height, GL_RGBA,
						GL_UNSIGNED_BYTE, buffer);
	u2gl_check_error("glReadPixels");

	snprintf(name, 80, "frame_%06d.raw", num++);
	f = fopen(name, "wb");
	if (f == NULL)
		return;
	
	int bytes_in_row = window_width * 4;
	int bytes_left = window_width * window_height * 4;
	while (bytes_left > 0) {
		int start_of_row = bytes_left - bytes_in_row;

		src = dest = &buffer[start_of_row];

		for (i = 0; i < window_width; i++) {
			unsigned char r = *src++;
			unsigned char g = *src++;
			unsigned char b = *src++;
			src++;

			*dest++ = r;
			*dest++ = g;
			*dest++ = b;
		}

		fwrite (&buffer[start_of_row], 1, window_width * 3, f);
		bytes_left -= bytes_in_row;
	}
	fclose (f);
}

