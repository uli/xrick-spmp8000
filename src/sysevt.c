/*
 * xrick/src/sysevt.c
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

/*
 * 20021010 SDLK_n replaced by SDLK_Fn because some non-US keyboards
 *          requires that SHIFT be pressed to input numbers.
 */

#include <libgame.h>

#include "system.h"
#include "config.h"
#include "game.h"
#include "debug.h"

#include "control.h"
#include "draw.h"

#include "syssnd.h"

#define SYSJOY_RANGE 3280

#define SETBIT(x,b) x |= (b)
#define CLRBIT(x,b) x &= ~(b)

emu_keymap_t keymap;

/*
 * Process an event
 */
static void
processEvent(uint32_t keys, uint32_t mask)
{
  if (keys & keymap.scancode[syskbd_up] || keys & keymap.scancode[EMU_KEY_UP]) {
    SETBIT(control_status, CONTROL_UP);
    control_last = CONTROL_UP;
  }
  if (keys & keymap.scancode[syskbd_down] || keys & keymap.scancode[EMU_KEY_DOWN]) {
    SETBIT(control_status, CONTROL_DOWN);
    control_last = CONTROL_DOWN;
  }
  if (keys & keymap.scancode[syskbd_left] || keys & keymap.scancode[EMU_KEY_LEFT]) {
    SETBIT(control_status, CONTROL_LEFT);
    control_last = CONTROL_LEFT;
  }
  if (keys & keymap.scancode[syskbd_right] || keys & keymap.scancode[EMU_KEY_RIGHT]) {
    SETBIT(control_status, CONTROL_RIGHT);
    control_last = CONTROL_RIGHT;
  }
  if (keys & keymap.scancode[syskbd_pause]) {
    SETBIT(control_status, CONTROL_PAUSE);
    control_last = CONTROL_PAUSE;
  }
  if (keys & keymap.scancode[syskbd_end]) {
    SETBIT(control_status, CONTROL_END);
    control_last = CONTROL_END;
  }
  if (keys & keymap.scancode[syskbd_xtra]) {
    SETBIT(control_status, CONTROL_EXIT);
    control_last = CONTROL_EXIT;
  }
  if (keys & keymap.scancode[syskbd_fire]) {
    SETBIT(control_status, CONTROL_FIRE);
    control_last = CONTROL_FIRE;
  }
  if (keys & keymap.scancode[EMU_KEY_ESC])
    NativeGE_gamePause();
  if (!(keys & keymap.scancode[syskbd_up] || keys & keymap.scancode[EMU_KEY_UP])) {
    CLRBIT(control_status, CONTROL_UP);
  }
  if (!(keys & keymap.scancode[syskbd_down] || keys & keymap.scancode[EMU_KEY_DOWN])) {
    CLRBIT(control_status, CONTROL_DOWN);
  }
  if (!(keys & keymap.scancode[syskbd_left] || keys & keymap.scancode[EMU_KEY_LEFT])) {
    CLRBIT(control_status, CONTROL_LEFT);
  }
  if (!(keys & keymap.scancode[syskbd_right] || keys & keymap.scancode[EMU_KEY_RIGHT])) {
    CLRBIT(control_status, CONTROL_RIGHT);
  }
  if (!(keys & keymap.scancode[syskbd_pause])) {
    CLRBIT(control_status, CONTROL_PAUSE);
  }
  if (!(keys & keymap.scancode[syskbd_end])) {
    CLRBIT(control_status, CONTROL_END);
  }
  if (!(keys & keymap.scancode[syskbd_xtra])) {
    CLRBIT(control_status, CONTROL_EXIT);
  }
  if (!(keys & keymap.scancode[syskbd_fire])) {
    CLRBIT(control_status, CONTROL_FIRE);
  }
#if 0
  switch (event_type) {
  case SDL_KEYDOWN:
    else if (key == SDLK_F1) {
      sysvid_toggleFullscreen();
    }
    else if (key == SDLK_F2) {
      sysvid_zoom(-1);
    }
    else if (key == SDLK_F3) {
      sysvid_zoom(+1);
    }
#ifdef ENABLE_SOUND
    else if (key == SDLK_F4) {
      syssnd_toggleMute();
    }
    else if (key == SDLK_F5) {
      syssnd_vol(-1);
    }
    else if (key == SDLK_F6) {
      syssnd_vol(+1);
    }
#endif
#ifdef ENABLE_CHEATS
    else if (key == SDLK_F7) {
      game_toggleCheat(1);
    }
    else if (key == SDLK_F8) {
      game_toggleCheat(2);
    }
    else if (key == SDLK_F9) {
      game_toggleCheat(3);
    }
#endif
    break;
  case SDL_KEYUP:
    break;
  case SDL_QUIT:
    /* player tries to close the window -- this is the same as pressing ESC */
    SETBIT(control_status, CONTROL_EXIT);
    control_last = CONTROL_EXIT;
    break;
#ifdef ENABLE_FOCUS
  case SDL_ACTIVEEVENT: {
    aevent = (SDL_ActiveEvent *)&event;
    IFDEBUG_EVENTS(
      printf("xrick/events: active %x %x\n", aevent->gain, aevent->state);
      );
    if (aevent->gain == 1)
      control_active = TRUE;
    else
      control_active = FALSE;
    }
  break;
#endif
#ifdef ENABLE_JOYSTICK
  case SDL_JOYAXISMOTION:
    IFDEBUG_EVENTS(sys_printf("xrick/events: joystick\n"););
    if (event.jaxis.axis == 0) {  /* left-right */
      if (event.jaxis.value < -SYSJOY_RANGE) {  /* left */
	SETBIT(control_status, CONTROL_LEFT);
	CLRBIT(control_status, CONTROL_RIGHT);
      }
      else if (event.jaxis.value > SYSJOY_RANGE) {  /* right */
	SETBIT(control_status, CONTROL_RIGHT);
	CLRBIT(control_status, CONTROL_LEFT);
      }
      else {  /* center */
	CLRBIT(control_status, CONTROL_RIGHT);
	CLRBIT(control_status, CONTROL_LEFT);
      }
    }
    if (event.jaxis.axis == 1) {  /* up-down */
      if (event.jaxis.value < -SYSJOY_RANGE) {  /* up */
	SETBIT(control_status, CONTROL_UP);
	CLRBIT(control_status, CONTROL_DOWN);
      }
      else if (event.jaxis.value > SYSJOY_RANGE) {  /* down */
	SETBIT(control_status, CONTROL_DOWN);
	CLRBIT(control_status, CONTROL_UP);
      }
      else {  /* center */
	CLRBIT(control_status, CONTROL_DOWN);
	CLRBIT(control_status, CONTROL_UP);
      }
    }
    break;
  case SDL_JOYBUTTONDOWN:
    SETBIT(control_status, CONTROL_FIRE);
    break;
  case SDL_JOYBUTTONUP:
    CLRBIT(control_status, CONTROL_FIRE);
    break;
#endif
  default:
    break;
  }
#endif
}

/*
 * Process events, if any, then return
 */
static uint32_t oldkeys = 0;
void
sysevt_poll(void)
{
  uint32_t keys = emuIfKeyGetInput(&keymap);
  processEvent(keys, keys ^ oldkeys);
  oldkeys = keys;
}

/*
 * Wait for an event, then process it and return
 */
void
sysevt_wait(void)
{
  uint32_t keys;
  do {
    syssnd_callback();
    keys = emuIfKeyGetInput(&keymap);
  } while (!(keys ^ oldkeys));
  processEvent(keys, keys ^ oldkeys);
  oldkeys = keys;
}

/* eof */
