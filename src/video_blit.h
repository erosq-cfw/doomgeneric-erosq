#ifndef VIDEO_BLIT_H
#define VIDEO_BLIT_H
#include <stdint.h>
#define DOOM_W 320
#define DOOM_H 200
#define PANEL_W 320
#define PANEL_H 240
#define LETTERBOX ((PANEL_H - DOOM_H) / 2)  /* 20 */
uint16_t xrgb_to_rgb565(uint32_t px);
void blit_letterbox(uint16_t *dst, int dst_stride_px, const uint32_t *src);
#endif
