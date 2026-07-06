/* Poll every /dev/input/event* non-blocking, translate via map_event(), and
 * queue results in a small ring that DG_GetKey drains one-per-call. The wheel
 * (EV_REL) is a discrete turn: enqueue a press immediately followed by a
 * release so Doom turns a step per detent instead of latching. */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/input.h>   /* struct input_event; also brings EV_KEY etc. */
#include "input_ev.h"
#include "input_map.h"

#define MAXFDS 10
#define RING   128

static int fds[MAXFDS];
static int nfds;

static struct { unsigned char key; int pressed; } ring[RING];
static int head, tail;   /* tail==head => empty */

static void push(unsigned char key, int pressed)
{
    int n = (head + 1) % RING;
    if (n == tail) return;            /* full: drop (input is best-effort) */
    ring[head].key = key; ring[head].pressed = pressed; head = n;
}

void input_open(void)
{
    char path[32];
    for (int i = 0; i < MAXFDS; i++) {
        snprintf(path, sizeof path, "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd >= 0) fds[nfds++] = fd;
    }
}

/* Drain all pending kernel events into the ring. */
static void drain(void)
{
    struct input_event ev;
    for (int i = 0; i < nfds; i++) {
        while (read(fds[i], &ev, sizeof ev) == (ssize_t)sizeof ev) {
            unsigned char key; int pressed;
            if (!map_event(ev.type, ev.code, ev.value, &key, &pressed)) continue;
            push(key, pressed);
            if (ev.type == EV_REL) push(key, 0);  /* discrete wheel tap: auto-release */
        }
    }
}

int input_poll(unsigned char *doomkey, int *pressed)
{
    if (tail == head) drain();
    if (tail == head) return 0;
    *doomkey = ring[tail].key; *pressed = ring[tail].pressed;
    tail = (tail + 1) % RING;
    return 1;
}
