#include <assert.h>
#include <string.h>
#include "sfx_mix.h"
int main(void)
{
    static const uint8_t a[3] = {128, 255, 0};   /* silence, max+, max- (unsigned) */
    static const uint8_t b[3] = {128, 255, 0};
    sfx_chan_t ch[SFX_CHANNELS]; memset(ch, 0, sizeof ch);
    ch[0] = (sfx_chan_t){a, 3, 0, 127, 1};
    ch[1] = (sfx_chan_t){b, 3, 0, 127, 1};
    int16_t out[3];
    sfx_mix(ch, out, 3);
    assert(out[0] == 0);                 /* two silences */
    assert(out[1] > out[0]);             /* two positive peaks sum, clamped */
    assert(out[2] < out[0]);             /* two negative peaks */
    assert(ch[0].active == 0 && ch[1].active == 0); /* consumed 3/3 -> done */

    /* empty channel set -> pure silence */
    sfx_chan_t z[SFX_CHANNELS]; memset(z, 0, sizeof z);
    int16_t s[4] = {9,9,9,9};
    sfx_mix(z, s, 4);
    for (int i = 0; i < 4; i++) assert(s[i] == 0);
    return 0;
}
