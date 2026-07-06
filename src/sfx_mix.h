#ifndef SFX_MIX_H
#define SFX_MIX_H
#include <stdint.h>
#define SFX_CHANNELS 8
typedef struct { const uint8_t *data; int len; int pos; int vol; int active; } sfx_chan_t;
void sfx_mix(sfx_chan_t *chans, int16_t *out, int frames);
#endif
