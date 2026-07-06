#include <assert.h>
#include <string.h>
#include "video_blit.h"
#define SW 320
#define SH 200
int main(void)
{
    /* pure white and pure red convert correctly */
    assert(xrgb_to_rgb565(0x00FFFFFF) == 0xFFFF);
    assert(xrgb_to_rgb565(0x00FF0000) == 0xF800); /* red */
    assert(xrgb_to_rgb565(0x0000FF00) == 0x07E0); /* green */
    assert(xrgb_to_rgb565(0x000000FF) == 0x001F); /* blue */
    assert(xrgb_to_rgb565(0x00000000) == 0x0000);

    static uint32_t src[SW*SH];
    for (int i = 0; i < SW*SH; i++) src[i] = 0x00FF0000; /* all red */
    static uint16_t dst[SW*240];
    memset(dst, 0xAA, sizeof dst);           /* poison to prove clearing */
    blit_letterbox(dst, SW, src);

    assert(dst[0] == 0x0000);                 /* top letterbox black */
    assert(dst[19*SW] == 0x0000);             /* last black row */
    assert(dst[20*SW + 0] == 0xF800);         /* first image row = red */
    assert(dst[219*SW + (SW-1)] == 0xF800);   /* last image pixel = red */
    assert(dst[220*SW] == 0x0000);            /* bottom letterbox black */
    return 0;
}
