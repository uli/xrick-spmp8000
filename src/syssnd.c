/*
 * xrick/src/syssnd.c
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
#include <stdlib.h>
//#include <memory.h>
#include <string.h>

#include "config.h"

#ifdef ENABLE_SOUND

#include "system.h"
#include "game.h"
#include "syssnd.h"
#include "debug.h"
#include "data.h"

static U8 isAudioActive = FALSE;

static U8 sndMute = FALSE;  /* mute flag */

typedef struct {
    char    *audio_data;
    int     audio_size;
} audio_data_desc;
ge_res_entry_t resources[30];

static void
end_channel(U8 c)
{
#if 0
	channel[c].loop = 0;
	if (channel[c].snd->dispose)
		syssnd_free(channel[c].snd);
	channel[c].snd = NULL;
#endif
}

void
syssnd_init(void)
{
#if 0
  for (c = 0; c < SYSSND_MIXCHANNELS; c++)
    channel[c].loop = 0;  /* deactivate */
#endif

	isAudioActive = TRUE;
  strcpy(resources[0].filename, "TAEND");
  resources[0].res_data = 0;
  NativeGE_initRes(0, resources);
}

/*
 * Shutdown
 */
void
syssnd_shutdown(void)
{
  if (!isAudioActive) return;

  isAudioActive = FALSE;
}

/*
 * Toggle mute
 *
 * When muted, sounds are still managed but not sent to the dsp, hence
 * it is possible to un-mute at any time.
 */
void
syssnd_toggleMute(void)
{
#if 0
  SDL_mutexP(sndlock);
  sndMute = !sndMute;
  SDL_mutexV(sndlock);
#endif
}

void
syssnd_vol(S8 d)
{
#if 0
  if ((d < 0 && sndUVol > 0) ||
      (d > 0 && sndUVol < SYSSND_MAXVOL)) {
    sndUVol += d;
    SDL_mutexP(sndlock);
    sndVol = SDL_MIX_MAXVOLUME * sndUVol / SYSSND_MAXVOL;
    SDL_mutexV(sndlock);
  }
#endif
}

/*
 * Play a sound
 *
 * loop: number of times the sound should be played, -1 to loop forever
 * returns: channel number, or -1 if none was available
 *
 * NOTE if sound is already playing, simply reset it (i.e. can not have
 * twice the same sound playing -- tends to become noisy when too many
 * bad guys die at the same time).
 */
S8
syssnd_play(sound_t *sound, S8 loop)
{
  audio_data_desc ri;
  int type = NativeGE_getRes(resources[sound->res_idx].filename, &ri);
  fprintf(stderr, "got res %s, type %d, size %d\n", resources[sound->res_idx].filename, type, ri.audio_size);
  int i;
  for (i = 0; i < 16; i++) {
  	fprintf(stderr, "%02x ", ri.audio_data[i]);
  }
  fprintf(stderr, "\n");
  int ret = NativeGE_playRes(type, 1, &ri);
  fprintf(stderr, "playres %d\n", ret);
  return 1;
}

/*
 * Pause
 *
 * pause: TRUE or FALSE
 * clear: TRUE to cleanup all sounds and make sure we start from scratch
 */
void
syssnd_pause(U8 pause, U8 clear)
{
#if 0
  U8 c;

  if (!isAudioActive) return;

  if (clear == TRUE) {
    SDL_mutexP(sndlock);
    for (c = 0; c < SYSSND_MIXCHANNELS; c++)
      channel[c].loop = 0;
    SDL_mutexV(sndlock);
  }

  if (pause == TRUE)
    SDL_PauseAudio(1);
  else
    SDL_PauseAudio(0);
#endif
}

/*
 * Stop a channel
 */
void
syssnd_stopchan(S8 chan)
{
#if 0
  if (chan < 0 || chan > SYSSND_MIXCHANNELS)
    return;

  SDL_mutexP(sndlock);
  if (channel[chan].snd) end_channel(chan);
  SDL_mutexV(sndlock);
#endif
}

/*
 * Stop a sound
 */
void
syssnd_stopsound(sound_t *sound)
{
#if 0
	U8 i;

	if (!sound) return;

	SDL_mutexP(sndlock);
	for (i = 0; i < SYSSND_MIXCHANNELS; i++)
		if (channel[i].snd == sound) end_channel(i);
	SDL_mutexV(sndlock);
#endif
}

/*
 * Stops all channels.
 */
void
syssnd_stopall(void)
{
#if 0
	U8 i;

	SDL_mutexP(sndlock);
	for (i = 0; i < SYSSND_MIXCHANNELS; i++)
		if (channel[i].snd) end_channel(i);
	SDL_mutexV(sndlock);
#endif
}

/*
 * Load a sound.
 */
sound_t *
syssnd_load(char *name)
{
	sound_t *s;

	/* open */
	data_file_t *f;
	if ((f = data_file_open(name)) == 0)
		return NULL;

	/* alloc sound */
	s = malloc(sizeof(sound_t));

	/* read */
	s->buf = malloc(1096820);
	s->len = data_file_read(f, s->buf, 1, 1096820);
	s->buf = realloc(s->buf, s->len);
	data_file_close(f);
	s->dispose = FALSE;

	int i;
	for (i = 0; strcmp(resources[i].filename, "TAEND"); i++) {
	}
	strcpy(resources[i].filename, rindex(name, '/'));
	resources[i].res_data = s->buf;
	s->res_idx = i;
	strcpy(resources[i + 1].filename, "TAEND");
	resources[i + 1].res_data = 0;
	fprintf(stderr, "loaded %s (%s, idx %d), %d bytes\n", name, resources[i].filename, s->res_idx, s->len);
	return s;
}

/*
 *
 */
void
syssnd_free(sound_t *s)
{
	if (!s) return;
	if (s->buf) free(s->buf);
	s->buf = NULL;
#ifdef DEBUG
	if (s->name) free(s->name);
	s->name = 0;
#endif
	s->len = 0;
}

#endif /* ENABLE_SOUND */

/* eof */

