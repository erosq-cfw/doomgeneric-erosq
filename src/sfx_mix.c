#include "sfx_mix.h"
void sfx_mix(sfx_chan_t *ch, int16_t *out, int frames)
{
    for (int f = 0; f < frames; f++) {
        int acc = 0;
        for (int c = 0; c < SFX_CHANNELS; c++) {
            if (!ch[c].active) continue;
            int s = ((int)ch[c].data[ch[c].pos] - 128) << 8; /* u8 -> s16 */
            acc += (s * ch[c].vol) / 127;
            if (++ch[c].pos >= ch[c].len) ch[c].active = 0;
        }
        if (acc > 32767) acc = 32767; else if (acc < -32768) acc = -32768;
        out[f] = (int16_t)acc;
    }
}
