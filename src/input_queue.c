#include "input_queue.h"
#include "input_map.h"

#define RING 128

static struct { unsigned char key; int pressed; } ring[RING];
static int head, tail;                 /* tail==head => empty */
static unsigned char defer[RING];      /* wheel releases held for the next cycle */
static int dcount;

static void ring_push(unsigned char key, int pressed)
{
    int n = (head + 1) % RING;
    if (n == tail) return;             /* full: drop (best-effort input) */
    ring[head].key = key; ring[head].pressed = pressed; head = n;
}

void iq_feed(uint16_t type, uint16_t code, int value)
{
    unsigned char key; int pressed;
    if (!map_event(type, code, value, &key, &pressed)) return;
    ring_push(key, pressed);
    if (type == EV_REL && dcount < RING) defer[dcount++] = key;  /* wheel: release next cycle */
}

int iq_poll(unsigned char *key, int *pressed)
{
    if (tail != head) {
        *key = ring[tail].key; *pressed = ring[tail].pressed;
        tail = (tail + 1) % RING;
        return 1;
    }
    /* Queue drained for this cycle. Promote any deferred wheel releases into the
     * ring so they surface on the NEXT poll cycle (next tic), then report empty
     * now so they aren't consumed in the current tic's drain. */
    if (dcount) {
        for (int i = 0; i < dcount; i++) ring_push(defer[i], 0);
        dcount = 0;
    }
    return 0;
}
