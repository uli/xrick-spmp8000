/*
 * xrick/src/scr_xrick.c
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

#include "system.h"
#include "game.h"
#include "screens.h"

#include "draw.h"
#include "control.h"
#include "img.h"

#include "img_splash.e"

#include "version.h"
/*
 * Display XRICK splash screen
 *
 * return: SCREEN_RUNNING, SCREEN_DONE, SCREEN_EXIT
 */
U8
screen_xrick(void)
{
	static U8 seq = 0;
	static U8 wait = 0;

	if (seq == 0) {
		sysvid_clear();
		draw_img(IMG_SPLASH);
		draw_tllst = (U8 *)"@SPMP8000@PORT@BY@\377"
		                   "@@@ULRICH@HECHT@@@\376";
		draw_setfb(80, 0);
		//draw_filter = 0x5555;
		draw_tilesList();
		draw_tllst = (U8 *)"R" BUILD_STRING "\376";
		draw_setfb(0, 0);
		draw_tilesList();
		game_rects = &draw_SCREENRECT;
		seq = 1;
	}

	switch (seq) {
	case 1:  /* wait */
		if (wait++ > 0x2) {
#ifdef ENABLE_SOUND
			game_setmusic("sounds/bullet.wav", 1);
#endif
			seq = 2;
			wait = 0;
		}
		break;

	case 2:  /* wait */
		if (wait++ > 0x60) {
			seq = 99;
			wait = 0;
		}
	}

	if (control_status & CONTROL_EXIT)  /* check for exit request */
		return SCREEN_EXIT;
        if (control_status & CONTROL_FIRE)
                seq = 99;

	if (seq == 99) {  /* we're done */
		sysvid_clear();
		sysvid_setGamePalette();
		seq = 0;
		return SCREEN_DONE;
	}

	return SCREEN_RUNNING;
}

/* eof */

