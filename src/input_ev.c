/* Device I/O half of input: open every /dev/input/event*, read raw events
 * non-blocking, and feed them to the pure input_queue (which owns mapping and
 * the wheel's cross-tic release timing). DG_GetKey -> input_poll -> iq_poll. */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/input.h>   /* struct input_event */
#include "input_ev.h"
#include "input_queue.h"

#define MAXFDS 10

static int fds[MAXFDS];
static int nfds;

void input_open(void)
{
    char path[32];
    for (int i = 0; i < MAXFDS; i++) {
        snprintf(path, sizeof path, "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd >= 0) fds[nfds++] = fd;
    }
}

static void drain_fds(void)
{
    struct input_event ev;
    for (int i = 0; i < nfds; i++)
        while (read(fds[i], &ev, sizeof ev) == (ssize_t)sizeof ev)
            iq_feed(ev.type, ev.code, ev.value);
}

int input_poll(unsigned char *doomkey, int *pressed)
{
    drain_fds();
    return iq_poll(doomkey, pressed);
}
