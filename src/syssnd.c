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
#include <string.h>

#include "config.h"

#ifdef ENABLE_SOUND

#include "system.h"
#include "game.h"
#include "syssnd.h"
#include "debug.h"
#include "data.h"

#define ADJVOL(S) (S) //(((S)*sndVol)/SDL_MIX_MAXVOLUME)

static U8 isAudioActive = FALSE;
static channel_t channel[SYSSND_MIXCHANNELS];

//static U8 sndVol = SDL_MIX_MAXVOLUME;  /* internal volume */
//static U8 sndUVol = SYSSND_MAXVOL;  /* user-selected volume */
static U8 sndMute = FALSE;  /* mute flag */

/*
 * prototypes
 */
static void end_channel(U8);

emu_sound_params_t sp;

/*
 * Callback -- this is also where all sound mixing is done
 *
 * Note: it may not be that much a good idea to do all the mixing here ; it
 * may be more efficient to mix samples every frame, or maybe everytime a
 * new sound is sent to be played. I don't know.
 */
U32 last_snd;
void syssnd_callback(void)
{
  /* Don't do anything until the last 20ms of sound are playing. */
  if (sys_gettime() - last_snd < 80 || !isAudioActive)
	return;
  U8 c;
  S16 s;
  U32 i;
  S16 *stream = (S16 *)sp.buf;
  int len = sp.buf_size / 2;

  for (i = 0; i < (U32)len; i++) {
    s = 0;
    for (c = 0; c < SYSSND_MIXCHANNELS; c++) {
      if (channel[c].loop != 0) {  /* channel is active */
	if (channel[c].len > 0) {  /* not ending */
	  s += ADJVOL(*channel[c].buf - 0x80);
	  channel[c].buf++;
	  channel[c].len--;
	}
	else {  /* ending */
	  if (channel[c].loop > 0) channel[c].loop--;
	  if (channel[c].loop) {  /* just loop */
	    IFDEBUG_AUDIO2(sys_printf("xrick/audio: channel %d - loop\n", c););
	    channel[c].buf = channel[c].snd->buf;
	    channel[c].len = channel[c].snd->len;
	    s += ADJVOL(*channel[c].buf - 0x80);
	    channel[c].buf++;
	    channel[c].len--;
	  }
	  else {  /* end for real */
	    IFDEBUG_AUDIO2(sys_printf("xrick/audio: channel %d - end\n", c););
	    end_channel(c);
	  }
	}
      }
    }
    if (sndMute)
      stream[i] = 0;
    else {
      stream[i] = s * 256;
    }
  }

  emuIfSoundPlay(&sp);
  last_snd = sys_gettime();
}

static void
end_channel(U8 c)
{
	channel[c].loop = 0;
	if (channel[c].snd->dispose)
		syssnd_free(channel[c].snd);
	channel[c].snd = NULL;
}

void
syssnd_init(void)
{
  U16 c;

  sp.rate = SYSSND_FREQ;
  sp.depth = 8; /* seems to be ignored */
  sp.channels = SYSSND_CHANNELS;
  /* 100ms buffer */
  sp.buf_size = SYSSND_FREQ * SYSSND_CHANNELS * 2 / 10;
  sp.buf = malloc(sp.buf_size);

  if (emuIfSoundInit(&sp) < 0) {
    IFDEBUG_AUDIO(
      sys_printf("xrick/audio: can not initialize audio subsystem\n");
      );
    return;
  }

  for (c = 0; c < SYSSND_MIXCHANNELS; c++)
    channel[c].loop = 0;  /* deactivate */

	isAudioActive = TRUE;
  last_snd = sys_gettime();
}

/*
 * Shutdown
 */
void
syssnd_shutdown(void)
{
  if (!isAudioActive) return;

  emuIfSoundCleanup(&sp);
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
  sndMute = !sndMute;
}

void
syssnd_vol(S8 d)
{
  (void)d;
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
  S8 c;

  if (!isAudioActive) return -1;
  if (sound == NULL) return -1;

  c = 0;
  while ((channel[c].snd != sound || channel[c].loop == 0) &&
	 channel[c].loop != 0 &&
	 c < SYSSND_MIXCHANNELS)
    c++;
  if (c == SYSSND_MIXCHANNELS)
    c = -1;

  IFDEBUG_AUDIO(
    if (channel[c].snd == sound && channel[c].loop != 0)
      sys_printf("xrick/sound: already playing %s on channel %d - resetting\n",
		 sound->name, c);
    else if (c >= 0)
      sys_printf("xrick/sound: playing %s on channel %d\n", sound->name, c);
    );

  //fprintf(stderr, "syssnd_play c %d\n", c);
  if (c >= 0) {
    channel[c].loop = loop;
    channel[c].snd = sound;
    channel[c].buf = sound->buf;
    channel[c].len = sound->len;
  }

  return c;
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
  U8 c;

  if (!isAudioActive) return;

  if (clear == TRUE) {
    for (c = 0; c < SYSSND_MIXCHANNELS; c++)
      channel[c].loop = 0;
  }

#if 0
  if (pause == TRUE)
    SDL_PauseAudio(1);
  else
    SDL_PauseAudio(0);
#else
  (void)pause;
#endif
}

/*
 * Stop a channel
 */
void
syssnd_stopchan(S8 chan)
{
  if (chan < 0 || chan > SYSSND_MIXCHANNELS)
    return;

  if (channel[chan].snd) end_channel(chan);
}

/*
 * Stop a sound
 */
void
syssnd_stopsound(sound_t *sound)
{
	U8 i;

	if (!sound) return;

	for (i = 0; i < SYSSND_MIXCHANNELS; i++)
		if (channel[i].snd == sound) end_channel(i);
}

/*
 * See if a sound is playing
 */
int
syssnd_isplaying(sound_t *sound)
{
	U8 i, playing;

	playing = 0;
	for (i = 0; i < SYSSND_MIXCHANNELS; i++)
		if (channel[i].snd == sound) playing = 1;
	return playing;
}


/*
 * Stops all channels.
 */
void
syssnd_stopall(void)
{
	U8 i;

	for (i = 0; i < SYSSND_MIXCHANNELS; i++)
		if (channel[i].snd) end_channel(i);
}

/*
 * Load a sound.
 */
sound_t *
syssnd_load(char *name)
{
	sound_t *s;

	/* open */
	data_file_t *f = data_file_open(name);
	if (!f)
		return NULL;

	/* alloc sound */
	s = calloc(1, sizeof(sound_t));
#ifdef DEBUG
	s->name = strdup(name);
#endif

	/* read */
	/* second param == 1 -> close source once read */
	s->buf = malloc(1096820);
	s->len = data_file_read(f, s->buf, 1, 1096820);
	if (s->len < 44)
	{
		data_file_close(f);
		free(s->buf);
		free(s);
		return NULL;
	}
	s->buf = realloc(s->buf, s->len);
	/* skip WAV header */
	s->buf += 44;
	s->len -= 44;

	s->dispose = FALSE;

	data_file_close(f);

	//fprintf(stderr, "name %s size %d\n", name, s->len);

	return s;
}

/*
 *
 */
void
syssnd_free(sound_t *s)
{
	if (!s) return;
	if (s->buf)
		free(s->buf - 44); /* remember the WAV header we skipped? */
	s->buf = NULL;
	s->len = 0;
}

#endif /* ENABLE_SOUND */

/* eof */

