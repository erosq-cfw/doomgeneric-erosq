/* erosq doomgeneric backend — stub (Task 1). Real DRM/input/sound land in
 * later tasks. Just enough to link and run the tick loop headless.
 *
 * NOTE: DG_ScreenBuffer is declared `extern pixel_t *DG_ScreenBuffer;` in
 * doomgeneric.h and DEFINED in doomgeneric/doomgeneric/doomgeneric.c
 * (`pixel_t* DG_ScreenBuffer = NULL;`). Do NOT redefine it here — a second
 * definition would either fail to link (strict) or silently alias the real
 * one (common-symbol tentative-definition merging), masking bugs.
 */
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "doomgeneric.h"
#include "drm_out.h"
#include "video_blit.h"
#include "input_ev.h"

void DG_Init(void) { if (drm_open() != 0) exit(1); input_open(); }

void DG_DrawFrame(void)
{
    int stride;
    uint16_t *fb = drm_pixels(&stride);
    if (fb) { blit_letterbox(fb, stride, DG_ScreenBuffer); drm_present(); }
}
void DG_SleepMs(uint32_t ms) { struct timespec t = { ms/1000, (long)(ms%1000)*1000000L }; nanosleep(&t, 0); }
uint32_t DG_GetTicksMs(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (uint32_t)(t.tv_sec*1000 + t.tv_nsec/1000000); }
int DG_GetKey(int *pressed, unsigned char *key) { return input_poll(key, pressed); }
void DG_SetWindowTitle(const char *title) { (void)title; }

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);
    for (;;) doomgeneric_Tick();
}
