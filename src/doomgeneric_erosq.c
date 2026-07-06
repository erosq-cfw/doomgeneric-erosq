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
#include <time.h>
#include "doomgeneric.h"

void DG_Init(void) {}
void DG_DrawFrame(void) {}
void DG_SleepMs(uint32_t ms) { struct timespec t = { ms/1000, (long)(ms%1000)*1000000L }; nanosleep(&t, 0); }
uint32_t DG_GetTicksMs(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (uint32_t)(t.tv_sec*1000 + t.tv_nsec/1000000); }
int DG_GetKey(int *pressed, unsigned char *key) { (void)pressed; (void)key; return 0; }
void DG_SetWindowTitle(const char *title) { (void)title; }

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);
    for (;;) doomgeneric_Tick();
}
