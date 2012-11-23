/*
 * xrick/src/system.c
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

#include <libgame.h>

#include <stdarg.h>   /* args for sys_panic */
#include <fcntl.h>    /* fcntl in sys_panic */
#include <stdio.h>    /* printf */
#include <stdlib.h>
#include <signal.h>

#include "system.h"
#include "syssnd.h"

/*
 * Panic
 */
void
sys_panic(char *err, ...)
{
  va_list argptr;

  /* change stdin to non blocking */
  /*fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);*/
  /* NOTE HPUX: use ... is it OK on Linux ? */
  /* fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~O_NDELAY); */

  /* prepare message */
  va_start(argptr, err);
  vfprintf(stderr, err, argptr);
  va_end(argptr);

  /* print message and die */
  exit(1);
}


/*
 * Print a message
 */
void
sys_printf(char *msg, ...)
{
  va_list argptr;

  /* change stdin to non blocking */
  /*fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);*/
  /* NOTE HPUX: use ... is it OK on Linux ? */
  /* fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~O_NDELAY); */

  /* prepare message */
  va_start(argptr, msg);
  vfprintf(stderr, msg, argptr);
  va_end(argptr);
}

/*
 * Return number of milliseconds elapsed since first call
 */
U32
sys_gettime(void)
{
  static U32 ticks_base = 0;
  U32 ticks;

  ticks = libgame_utime() / 1000;

  if (!ticks_base)
    ticks_base = ticks;

  return ticks - ticks_base;
}

/*
 * Sleep a number of milliseconds
 */
void
sys_sleep(int s)
{
  U32 tm = sys_gettime();
  while (sys_gettime() - tm < (U32)s) {
    syssnd_callback();
  }
}

static int my_shutdown(uint32_t arg)
{
  (void)arg;
  sys_shutdown();
  fclose(stderr);
  return NativeGE_gameExit();
}

/*
 * Initialize system
 */
void
sys_init(int argc, char **argv)
{
        libgame_set_debug(0);
	sysarg_init(argc, argv);
	sysvid_init();
#ifdef ENABLE_JOYSTICK
	sysjoy_init();
#endif
#ifdef ENABLE_SOUND
	if (sysarg_args_nosound == 0)
		syssnd_init();
#endif
        g_stEmuAPIs->exit = my_shutdown;
//	signal(SIGINT, exit);
//	signal(SIGTERM, exit);
}

/*
 * Shutdown system
 */
void
sys_shutdown(void)
{
#ifdef ENABLE_SOUND
	syssnd_shutdown();
#endif
#ifdef ENABLE_JOYSTICK
	sysjoy_shutdown();
#endif
	sysvid_shutdown();
}

/* eof */
