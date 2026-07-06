/* erosq SFX backend: doomgeneric's sound_module_t backed by our tested mixer
 * (sfx_mix) writing S16 mono PCM to tinyalsa. Music is a no-op stub this phase
 * (deferred). Must be built with -DFEATURE_SOUND so i_sound.c links against the
 * DG_sound_module / DG_music_module symbols defined here (see INTERFACES.md).
 *
 * Rate: PCM runs at the native DMX SFX rate (11025 Hz), so the common case is
 * zero-copy — the channel points straight at the cached lump samples and the
 * 1:1 mixer needs no resampling. The rare non-11025 lump (a few 22050 sounds)
 * is nearest-neighbour resampled into a side buffer. If the codec rejects
 * 11025 on hardware (HITL), raise OUT_RATE to a rate it accepts; the resample
 * path then covers every sound (at more RAM) — measure it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "doomtype.h"
#include "i_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "sfx_mix.h"
#include "tinyalsa/asoundlib.h"

#define OUT_RATE   11025        /* ponytail: native DMX rate; HITL-tunable */
#define TICRATE    35           /* doom sound tics/sec */
#define TIC_FRAMES (OUT_RATE / TICRATE)   /* ~315 frames written per Update() */

/* i_sound.c's I_BindSoundVariables binds these two config vars under
 * FEATURE_SOUND; they normally live in the (excluded) SDL module. We don't
 * resample via libsamplerate, so they're inert — define them to satisfy the
 * linker. */
int   use_libsamplerate = 0;
float libsamplerate_scale = 0.65f;

static struct pcm *pcm;
static int use_prefix;
static sfx_chan_t chans[SFX_CHANNELS];
static uint8_t   *owned[SFX_CHANNELS];    /* resampled buffers to free (else NULL) */

/* --- sound_module_t implementation --------------------------------------- */

static boolean I_EQ_Init(boolean use_sfx_prefix)
{
    use_prefix = use_sfx_prefix;
    struct pcm_config cfg = {0};
    cfg.channels = 1;
    cfg.rate = OUT_RATE;
    cfg.format = PCM_FORMAT_S16_LE;
    cfg.period_size = 1024;
    cfg.period_count = 4;
    pcm = pcm_open(0, 0, PCM_OUT, &cfg);
    if (!pcm || !pcm_is_ready(pcm)) {
        if (pcm) pcm_close(pcm);
        pcm = NULL;
        return false;                     /* not selected -> game runs silent */
    }
    return true;
}

static void I_EQ_Shutdown(void)
{
    for (int c = 0; c < SFX_CHANNELS; c++) { free(owned[c]); owned[c] = NULL; }
    if (pcm) { pcm_close(pcm); pcm = NULL; }
}

static int I_EQ_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char buf[16];
    /* ponytail: vanilla name only (no dehacked substitution) — DOOM1 has none */
    if (use_prefix) snprintf(buf, sizeof buf, "ds%s", sfx->name);
    else            snprintf(buf, sizeof buf, "%s", sfx->name);
    return W_GetNumForName(buf);
}

static void I_EQ_StartChannel(int channel, const uint8_t *samples, int n, int rate, int vol)
{
    free(owned[channel]); owned[channel] = NULL;
    if (rate == OUT_RATE) {
        chans[channel] = (sfx_chan_t){ samples, n, 0, vol, 1 };  /* zero-copy */
    } else {
        long outn = (long)n * OUT_RATE / rate;
        uint8_t *buf = malloc(outn);
        if (!buf) { chans[channel].active = 0; return; }
        for (long i = 0; i < outn; i++) buf[i] = samples[i * rate / OUT_RATE];
        owned[channel] = buf;
        chans[channel] = (sfx_chan_t){ buf, (int)outn, 0, vol, 1 };
    }
}

static int I_EQ_StartSound(sfxinfo_t *sfx, int channel, int vol, int sep)
{
    (void)sep;                            /* mono out: no stereo separation */
    if (channel < 0 || channel >= SFX_CHANNELS || !pcm) return -1;
    int lumpnum = sfx->lumpnum > 0 ? sfx->lumpnum : I_EQ_GetSfxLumpNum(sfx);
    byte *data = W_CacheLumpNum(lumpnum, PU_STATIC);
    unsigned int lumplen = W_LumpLength(lumpnum);
    /* DMX lump: [0]=0x03 [1]=0x00, [2..3]=rate(LE16), [4..7]=len(LE32); DMX
     * skips the first/last 16 sample bytes (see chocolate-doom CacheSFX). */
    if (lumplen < 8 || data[0] != 0x03 || data[1] != 0x00) return -1;
    int rate = (data[3] << 8) | data[2];
    unsigned int length = (data[7]<<24)|(data[6]<<16)|(data[5]<<8)|data[4];
    if (rate <= 0) return -1;                     /* guard divide-by-zero in resample path */
    if (length > lumplen - 8 || length <= 48) return -1;
    I_EQ_StartChannel(channel, data + 24, (int)(length - 32), rate, vol);
    return channel;
}

static void I_EQ_UpdateSoundParams(int channel, int vol, int sep)
{
    (void)sep;
    if (channel >= 0 && channel < SFX_CHANNELS) chans[channel].vol = vol;
}

static void I_EQ_StopSound(int channel)
{
    if (channel >= 0 && channel < SFX_CHANNELS) chans[channel].active = 0;
}

static boolean I_EQ_SoundIsPlaying(int channel)
{
    if (channel < 0 || channel >= SFX_CHANNELS) return false;
    return chans[channel].active;
}

static void I_EQ_Update(void)
{
    if (!pcm) return;
    int16_t buf[TIC_FRAMES];
    sfx_mix(chans, buf, TIC_FRAMES);
    pcm_writei(pcm, buf, TIC_FRAMES);     /* blocks -> natural backpressure */
}

static void I_EQ_CacheSounds(sfxinfo_t *sounds, int num) { (void)sounds; (void)num; }

static snddevice_t sfx_devices[] = { SNDDEVICE_SB };  /* must include snd_sfxdevice (default SB) */

sound_module_t DG_sound_module =
{
    sfx_devices,
    sizeof(sfx_devices) / sizeof(*sfx_devices),
    I_EQ_Init,
    I_EQ_Shutdown,
    I_EQ_GetSfxLumpNum,
    I_EQ_Update,
    I_EQ_UpdateSoundParams,
    I_EQ_StartSound,
    I_EQ_StopSound,
    I_EQ_SoundIsPlaying,
    I_EQ_CacheSounds,
};

/* --- music_module_t: no-op stub (music deferred to a later phase) --------- */

static boolean M_Init(void) { return false; }   /* false -> doom disables music */
static void    M_Shutdown(void) {}
static void    M_SetVolume(int v) { (void)v; }
static void    M_Pause(void) {}
static void    M_Resume(void) {}
static void   *M_Register(void *d, int l) { (void)d; (void)l; return NULL; }
static void    M_UnRegister(void *h) { (void)h; }
static void    M_Play(void *h, boolean loop) { (void)h; (void)loop; }
static void    M_Stop(void) {}
static boolean M_IsPlaying(void) { return false; }
static void    M_Poll(void) {}

music_module_t DG_music_module =
{
    NULL, 0,
    M_Init, M_Shutdown, M_SetVolume, M_Pause, M_Resume,
    M_Register, M_UnRegister, M_Play, M_Stop, M_IsPlaying, M_Poll,
};
