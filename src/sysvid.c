/*
 * xrick/src/sysvid.c
 *
 * Copyright (C) 1998-2002 BigOrno (bigorno@bigorno.net). All rights reserved.
 *
 * The use and distribution terms for this software are contained in the file
 * named README, which can be found in the root of this distribution. By
 * using this software in any fashion, you are agreeing to be bound by the
 * terms of this license.
 *
 * You must not remove this notice, or any other, from this software.
 */

#include <stdlib.h> /* malloc */

#include <libgame.h>
#include <string.h>

#include "system.h"
#include "game.h"
#include "img.h"
#include "debug.h"

#ifdef __MSVC__
#include <memory.h> /* memset */
#endif

U8 *sysvid_fb; /* frame buffer */
rect_t SCREENRECT = {0, 0, SYSVID_WIDTH, SYSVID_HEIGHT, NULL}; /* whole fb */

static uint16_t palette[256];
static emu_graph_params_t gp;

static U8 zoom = SYSVID_ZOOM; /* actual zoom level */
static U8 szoom = 0;  /* saved zoom level */
static U8 fszoom = 0;  /* fullscreen zoom level */

#include "img_icon.e"

/*
 * color tables
 */

#ifdef GFXPC
static U8 RED[] = { 0x00, 0x50, 0xf0, 0xf0, 0x00, 0x50, 0xf0, 0xf0 };
static U8 GREEN[] = { 0x00, 0xf8, 0x50, 0xf8, 0x00, 0xf8, 0x50, 0xf8 };
static U8 BLUE[] = { 0x00, 0x50, 0x50, 0x50, 0x00, 0xf8, 0xf8, 0xf8 };
#endif
#ifdef GFXST
static U8 RED[] = { 0x00, 0xd8, 0xb0, 0xf8,
                    0x20, 0x00, 0x00, 0x20,
                    0x48, 0x48, 0x90, 0xd8,
                    0x48, 0x68, 0x90, 0xb0,
                    /* cheat colors */
                    0x50, 0xe0, 0xc8, 0xf8,
                    0x68, 0x50, 0x50, 0x68,
                    0x80, 0x80, 0xb0, 0xe0,
                    0x80, 0x98, 0xb0, 0xc8
};
static U8 GREEN[] = { 0x00, 0x00, 0x6c, 0x90,
                      0x24, 0x48, 0x6c, 0x48,
                      0x6c, 0x24, 0x48, 0x6c,
                      0x48, 0x6c, 0x90, 0xb4,
		      /* cheat colors */
                      0x54, 0x54, 0x9c, 0xb4,
                      0x6c, 0x84, 0x9c, 0x84,
                      0x9c, 0x6c, 0x84, 0x9c,
                      0x84, 0x9c, 0xb4, 0xcc
};
static U8 BLUE[] = { 0x00, 0x00, 0x68, 0x68,
                     0x20, 0xb0, 0xd8, 0x00,
                     0x20, 0x00, 0x00, 0x00,
                     0x48, 0x68, 0x90, 0xb0,
		     /* cheat colors */
                     0x50, 0x50, 0x98, 0x98,
                     0x68, 0xc8, 0xe0, 0x50,
                     0x68, 0x50, 0x50, 0x50,
                     0x80, 0x98, 0xb0, 0xc8};
#endif

/*
 * Initialize screen
 */
static
void initScreen(U16 w, U16 h)
{
  gp.pixels = malloc(w * h * 2);
  gp.width = w;
  gp.height = h;
  gp.unknown_flag = 0;
  gp.src_clip_x = 0;
  gp.src_clip_y = 0;
  gp.src_clip_w = w;
  gp.src_clip_h = h;
  emuIfGraphInit(&gp);
}

void
sysvid_setPalette(img_color_t *pal, U16 n)
{
  U16 i;

  for (i = 0; i < n; i++) {
    palette[i] = MAKE_RGB565(pal[i].r, pal[i].g, pal[i].b);
  }
}

void
sysvid_restorePalette()
{
}

void
sysvid_setGamePalette()
{
  U8 i;
  img_color_t pal[256];

  for (i = 0; i < 32; ++i) {
    pal[i].r = RED[i];
    pal[i].g = GREEN[i];
    pal[i].b = BLUE[i];
  }
  sysvid_setPalette(pal, 32);
}

/*
 * Initialize video modes
 */
void
sysvid_chkvm(void)
{
  fszoom = 1;
}

extern emu_keymap_t keymap;
/*
 * Initialise video
 */
void
sysvid_init(void)
{
  IFDEBUG_VIDEO(sys_printf("xrick/video: start\n"););

  /* video modes and screen */
  sysvid_chkvm();  /* check video modes */
  if (sysarg_args_zoom)
    zoom = sysarg_args_zoom;
  if (sysarg_args_fullscreen) {
    szoom = zoom;
    zoom = fszoom;
  }
  initScreen(SYSVID_WIDTH * zoom,
		      SYSVID_HEIGHT * zoom);

  /*
   * create v_ frame buffer
   */
  sysvid_fb = malloc(SYSVID_WIDTH * SYSVID_HEIGHT);
  if (!sysvid_fb)
    sys_panic("xrick/video: sysvid_fb malloc failed\n");

  IFDEBUG_VIDEO(sys_printf("xrick/video: ready\n"););
  
  emuIfKeyInit(&keymap);
}

/*
 * Shutdown video
 */
void
sysvid_shutdown(void)
{
  free(sysvid_fb);
  sysvid_fb = NULL;

  emuIfGraphCleanup();
  emuIfKeyCleanup(&keymap);
}

/*
 * Update screen
 * NOTE errors processing ?
 */
void
sysvid_update(rect_t *rects)
{
  U16 x, y, xz, yz;
  U8 *p, *p0;
  uint16_t *q;
  uint16_t *q0;

  if (rects == NULL)
    return;

  while (rects) {
    p0 = sysvid_fb;
    p0 += rects->x + rects->y * SYSVID_WIDTH;
    q0 = gp.pixels;
    q0 += (rects->x + rects->y * SYSVID_WIDTH * zoom) * zoom;

    for (y = rects->y; y < rects->y + rects->height; y++) {
      for (yz = 0; yz < zoom; yz++) {
	p = p0;
	q = q0;
	for (x = rects->x; x < rects->x + rects->width; x++) {
	  for (xz = 0; xz < zoom; xz++) {
	    *q = palette[*p];
	    q++;
	  }
	  p++;
	}
	q0 += SYSVID_WIDTH * zoom;
      }
      p0 += SYSVID_WIDTH;
    }

    IFDEBUG_VIDEO2(
    for (y = rects->y; y < rects->y + rects->height; y++)
      for (yz = 0; yz < zoom; yz++) {
	p = (U8 *)screen->pixels + rects->x * zoom + (y * zoom + yz) * SYSVID_WIDTH * zoom;
	*p = 0x01;
	*(p + rects->width * zoom - 1) = 0x01;
      }

    for (x = rects->x; x < rects->x + rects->width; x++)
      for (xz = 0; xz < zoom; xz++) {
	p = (U8 *)screen->pixels + x * zoom + xz + rects->y * zoom * SYSVID_WIDTH * zoom;
	*p = 0x01;
	*(p + ((rects->height * zoom - 1) * zoom) * SYSVID_WIDTH) = 0x01;
      }
    );

    rects = rects->next;
  }
  emuIfGraphShow();
}


/*
 * Clear screen
 * (077C)
 */
void
sysvid_clear(void)
{
  memset(sysvid_fb, 0, SYSVID_WIDTH * SYSVID_HEIGHT);
}


/*
 * Zoom
 */
void
sysvid_zoom(S8 z)
{
  (void)z;
}

/*
 * Toggle fullscreen
 */
void
sysvid_toggleFullscreen(void)
{
}

/* eof */



