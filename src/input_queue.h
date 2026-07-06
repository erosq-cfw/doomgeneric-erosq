#ifndef INPUT_QUEUE_H
#define INPUT_QUEUE_H
#include <stdint.h>
/* Pure event queue between the raw input devices and DG_GetKey. Kept free of
 * file descriptors so the cross-tic release timing can be unit-tested.
 *
 * iq_feed(): translate one raw event (via input_map) and enqueue it.
 * iq_poll(): return one queued (key,pressed) or 0 when the queue is drained.
 *
 * The wheel (EV_REL) is a discrete detent: iq_feed enqueues the PRESS now and
 * schedules the matching RELEASE for the NEXT poll cycle. doomgeneric's
 * I_GetEvent drains DG_GetKey to empty every tic and samples key state
 * afterwards, so a same-cycle press+release would cancel out (net zero turn).
 * Deferring the release by one cycle holds the turn key down for exactly one
 * tic. Real buttons are held across many tics by the hardware, so they need no
 * deferral and pass straight through. */
void iq_feed(uint16_t type, uint16_t code, int value);
int  iq_poll(unsigned char *key, int *pressed);
#endif
