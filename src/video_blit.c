#include "video_blit.h"
uint16_t xrgb_to_rgb565(uint32_t px)
{
    uint32_t r = (px >> 16) & 0xFF, g = (px >> 8) & 0xFF, b = px & 0xFF;
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}
void blit_letterbox(uint16_t *dst, int stride, const uint32_t *src)
{
    for (int y = 0; y < LETTERBOX; y++)
        for (int x = 0; x < PANEL_W; x++) dst[y*stride + x] = 0;
    for (int y = 0; y < DOOM_H; y++)
        for (int x = 0; x < DOOM_W; x++)
            dst[(y + LETTERBOX)*stride + x] = xrgb_to_rgb565(src[y*DOOM_W + x]);
    for (int y = LETTERBOX + DOOM_H; y < PANEL_H; y++)
        for (int x = 0; x < PANEL_W; x++) dst[y*stride + x] = 0;
}
