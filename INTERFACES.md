# doomgeneric interfaces captured for later tasks

Vendored from upstream `https://github.com/ozkl/doomgeneric.git`, commit
present at clone time (Task 1, 2026-07): pinned resolution 320x200 via
sed in `build.sh`. tinyalsa pinned at tag `1.1.1` (commit `df11091086b5`,
same commit erosq-ui already vendors).

All paths below are relative to `doom/doomgeneric/doomgeneric/`.

## DG_* callbacks — `doomgeneric.h`

```c
#ifndef DOOMGENERIC_RESX
#define DOOMGENERIC_RESX 640
#endif  // DOOMGENERIC_RESX

#ifndef DOOMGENERIC_RESY
#define DOOMGENERIC_RESY 400
#endif  // DOOMGENERIC_RESY

#ifdef CMAP256
typedef uint8_t pixel_t;
#else  // CMAP256
typedef uint32_t pixel_t;
#endif  // CMAP256

extern pixel_t* DG_ScreenBuffer;

void doomgeneric_Create(int argc, char **argv);
void doomgeneric_Tick();

//Implement below functions for your platform
void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);
```

`build.sh` sed-pins `DOOMGENERIC_RESX`/`DOOMGENERIC_RESY` to `320`/`200`
(letterboxed into the 320x240 panel — later display task centers with a
20px top/bottom band).

**`DG_ScreenBuffer` DEFINITION SITE — CONFIRMED**: upstream defines the
actual storage in `doomgeneric.c`, not in the header (the header only has
the `extern` declaration above):

```c
// doomgeneric.c, line 7
pixel_t* DG_ScreenBuffer = NULL;
```

and it is allocated inside `doomgeneric_Create`:

```c
// doomgeneric.c, doomgeneric_Create()
DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
```

Consequence for `src/doomgeneric_erosq.c`: the stub/backend **must not**
declare `uint32_t *DG_ScreenBuffer = 0;` (the brief's draft stub had this
line) — that would be a second definition of the same global. It was
**deleted** from the committed stub. Later display-backend code should
just `#include "doomgeneric.h"` and read the existing `extern`
`DG_ScreenBuffer` (type is `pixel_t*`, which is `uint32_t*` unless
`CMAP256` is defined — we do not define `CMAP256`, so it's `uint32_t*`,
0xAARRGGBB per pixel going by upstream renderer usage).

Also note: `doomgeneric_Tick()` is **not** defined in `doomgeneric.c`.
It's defined in `d_main.c`:

```c
// d_main.c, line 405
void doomgeneric_Tick()
{
    I_StartFrame ();
    TryRunTics ();
    S_UpdateSounds (players[consoleplayer].mo);
    if (screenvisible)
    {
        D_Display ();
    }
}
```

## doomgeneric_Create / doomgeneric_Tick prototypes

Both declared in `doomgeneric.h` with old-style empty-parens (K&R,
unspecified-argument) prototypes — copy exactly, do not add `(void)`:

```c
void doomgeneric_Create(int argc, char **argv);
void doomgeneric_Tick();
```

`doomgeneric_Create` is defined in `doomgeneric.c`; it saves argv,
allocates `DG_ScreenBuffer`, calls `DG_Init()`, then calls `D_DoomMain()`
(which never returns in the reference callers — the platform main is
expected to loop calling `doomgeneric_Tick()` itself, see the stub's
`for (;;) doomgeneric_Tick();` in `main()`).

## sound_module_t / snddevice_t / sfxinfo_t — `i_sound.h`

```c
typedef struct sfxinfo_struct	sfxinfo_t;

struct sfxinfo_struct
{
    char *tagname;          // tag name, used for hexen.
    char name[9];           // lump name ('DS'/'DP' prefix if use_sfx_prefix)
    int priority;           // Sfx priority
    sfxinfo_t *link;        // referenced sound if a link
    int pitch;              // pitch if a link
    int volume;             // volume if a link
    int usefulness;         // checked every second; 0 decrement, -1 throw
                             // out, >0 in use
    int lumpnum;            // lump number of sfx
    int numchannels;        // max channels sound can play on (Heretic)
    void *driver_data;      // data used by the low level code
};

typedef enum
{
    SNDDEVICE_NONE = 0,
    SNDDEVICE_PCSPEAKER = 1,
    SNDDEVICE_ADLIB = 2,
    SNDDEVICE_SB = 3,
    SNDDEVICE_PAS = 4,
    SNDDEVICE_GUS = 5,
    SNDDEVICE_WAVEBLASTER = 6,
    SNDDEVICE_SOUNDCANVAS = 7,
    SNDDEVICE_GENMIDI = 8,
    SNDDEVICE_AWE32 = 9,
    SNDDEVICE_CD = 10,
} snddevice_t;

// Interface for sound modules
typedef struct
{
    snddevice_t *sound_devices;
    int num_sound_devices;

    boolean (*Init)(boolean use_sfx_prefix);
    void (*Shutdown)(void);
    int (*GetSfxLumpNum)(sfxinfo_t *sfxinfo);
    void (*Update)(void);
    void (*UpdateSoundParams)(int channel, int vol, int sep);
    int (*StartSound)(sfxinfo_t *sfxinfo, int channel, int vol, int sep);
    void (*StopSound)(int channel);
    boolean (*SoundIsPlaying)(int channel);
    void (*CacheSounds)(sfxinfo_t *sounds, int num_sounds);
} sound_module_t;
```

Music side (Task 7 likely also needs this — same header):

```c
typedef struct
{
    snddevice_t *sound_devices;
    int num_sound_devices;

    boolean (*Init)(void);
    void (*Shutdown)(void);
    void (*SetMusicVolume)(int volume);
    void (*PauseMusic)(void);
    void (*ResumeMusic)(void);
    void *(*RegisterSong)(void *data, int len);
    void (*UnRegisterSong)(void *handle);
    void (*PlaySong)(void *handle, boolean looping);
    void (*StopSong)(void);
    boolean (*MusicIsPlaying)(void);
    void (*Poll)(void);
} music_module_t;
```

`boolean` comes from `doomtype.h` (an `int`-based enum-ish typedef, not
C99 `_Bool` — standard Chocolate Doom convention).

### Hook points for a new sound backend (`i_sound.h` tail)

```c
void I_InitTimidityConfig(void);
#ifdef FEATURE_SOUND
extern sound_module_t DG_sound_module;
extern music_module_t DG_music_module;
#endif
extern sound_module_t sound_pcsound_module;
extern music_module_t music_opl_module;
```

This is the important bit for Task 7: to plug in a real (tinyalsa-backed)
sound module, define a `sound_module_t DG_sound_module` and
`music_module_t DG_music_module` (exact names, matched by `i_sound.c`,
see below) somewhere linked into the build, and compile everything with
`-DFEATURE_SOUND`. `doomfeatures.h` currently has it commented out:
`//#undef FEATURE_SOUND` — i.e. FEATURE_SOUND is undefined by default and
must be supplied via `-DFEATURE_SOUND` on the command line (adding
`-UFEATURE_SOUND`/editing the header is not required).

## sound_modules[] / music_modules[] — `i_sound.c`

There is only a `sound_modules[]` array (no separate `music_modules[]`
array — music uses a single static pointer, see below). Declared as a
static array of pointers, gated by `FEATURE_SOUND`:

```c
// Compiled-in sound modules:
static sound_module_t *sound_modules[] =
{
    #ifdef FEATURE_SOUND
    &DG_sound_module,
    #endif
    NULL,
};
```

`InitSfxModule()` walks `sound_modules[]` until `sound_modules[i] == NULL`,
picks the first module whose `sound_devices` list contains `snd_sfxdevice`
and whose `Init()` returns true, and stores the winner in a static
`sound_module_t *sound_module`. **Consequence**: to be selected,
`DG_sound_module.sound_devices` must include whatever `snd_sfxdevice`
resolves to (default `SNDDEVICE_SB` per `i_sound.c`, see
`I_BindSoundVariables`/config) — Task 7 should populate
`DG_sound_module.sound_devices` with a list that includes `SNDDEVICE_SB`
(or otherwise make sure the configured `snd_sfxdevice` is in that list).

Music module selection is unconditional (no list-walk, no device
matching), just a straight assignment when `FEATURE_SOUND` is defined:

```c
static void InitMusicModule(void)
{
#ifdef FEATURE_SOUND
    music_module = &DG_music_module;
#endif /* FEATURE_SOUND */
}
```

So Task 7 must define exactly two extern symbols —
`sound_module_t DG_sound_module` and `music_module_t DG_music_module` —
and build with `-DFEATURE_SOUND`.

## doomkeys.h — KEY_STRAFE_L / KEY_STRAFE_R

**They exist upstream** (no fallback to `,`/`.` needed):

```c
#define KEY_STRAFE_L	0xa0
#define KEY_STRAFE_R	0xa1
```

(also present, for Task 4's reference: `KEY_RIGHTARROW 0xae`,
`KEY_LEFTARROW 0xac`, `KEY_UPARROW 0xad`, `KEY_DOWNARROW 0xaf`,
`KEY_USE 0xa2`, `KEY_FIRE 0xa3`, `KEY_ESCAPE 27`, `KEY_ENTER 13`,
`KEY_TAB 9`.)

## Repo-drift note: extra platform files not in the original SKIP list

The brief's `Makefile.erosq` draft only excluded
`doomgeneric_xlib doomgeneric_sdl doomgeneric_win doomgeneric_soso
doomgeneric_allegro`. The current upstream `doomgeneric/doomgeneric/`
directory (as of this clone) has **more** platform files than that list
covers, and building without excluding them fails:

- Extra platform mains (each defines its own `DG_Init`/`DG_DrawFrame`/
  `main`, conflicting with `src/doomgeneric_erosq.c`):
  `doomgeneric_emscripten.c`, `doomgeneric_linuxvt.c`,
  `doomgeneric_sosox.c`.
- SDL/allegro **sound/music** backend files that are not `doomgeneric_*`
  mains but still `#include <SDL.h>` / `#include <allegro/base.h>`
  unconditionally (no `#ifdef` guard around the include) and fail to
  compile with no SDL/allegro dev headers installed:
  `i_sdlmusic.c`, `i_sdlsound.c`, `i_allegromusic.c`, `i_allegrosound.c`.

`doom/Makefile.erosq`'s `SKIP` list was extended to include all of the
above (8 basenames total, up from 5) so the wildcard-minus-SKIP source
list builds cleanly. If doomgeneric upstream grows further platform
backends in the future, re-run `build.sh` and add any new failures to
`SKIP`.

## tinyalsa 1.1.1 / musl note

Vendoring `tinyalsa` tag `1.1.1` (pinned commit `df11091086b5`, matching
what `erosq-ui/tinyalsa` already vendors) fails to compile as-is under
the musl toolchain: `tinyalsa/include/tinyalsa/interval.h` uses
`ssize_t` but only includes `<stdlib.h>`, which pulls in `ssize_t`
transitively under glibc but not under musl. `erosq-ui/tinyalsa` already
carries a local (uncommitted) one-line fix on this exact commit adding
`#include <sys/types.h>`. `doom/build.sh` now applies the identical fix
idempotently (via `sed`, guarded by a `grep -q` check) right after
cloning, before invoking `make -C tinyalsa/src`.
