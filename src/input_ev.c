/* Device I/O half of input: open every /dev/input/event*, read raw events
 * non-blocking, and feed them to the pure input_queue (which owns mapping and
 * the wheel's cross-tic release timing). DG_GetKey -> input_poll -> iq_poll. */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <linux/input.h>   /* struct input_event */
#include "input_ev.h"
#include "input_queue.h"

#define MAXFDS 10

/* Slot i is /dev/input/eventi, or -1 if not open. Indexing by node number (not
 * packing into nfds) is what lets a device come and go: a BT pad's node vanishes
 * on disconnect and the kernel does NOT close our fd, it just fails with ENODEV
 * forever, while the reconnect creates a new node at the same path. Without the
 * reopen below, unplugging the pad killed it until doom was restarted. */
static int fds[MAXFDS];

static void open_missing(void)
{
    char path[32];
    for (int i = 0; i < MAXFDS; i++) {
        if (fds[i] >= 0) continue;
        snprintf(path, sizeof path, "/dev/input/event%d", i);
        fds[i] = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    }
}

void input_open(void)
{
    for (int i = 0; i < MAXFDS; i++) fds[i] = -1;
    open_missing();
}

static void drain_fds(void)
{
    static unsigned rescan;
    struct input_event ev;
    for (int i = 0; i < MAXFDS; i++) {
        ssize_t n;
        if (fds[i] < 0) continue;
        while ((n = read(fds[i], &ev, sizeof ev)) == (ssize_t)sizeof ev)
            iq_feed(ev.type, ev.code, ev.value);
        if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            close(fds[i]);      /* device went away (ENODEV) */
            fds[i] = -1;
        }
    }
    /* ~1s at any sane tic rate; probing 10 paths every tic would be silly */
    if ((++rescan & 63) == 0)
        open_missing();
}

int input_poll(unsigned char *doomkey, int *pressed)
{
    drain_fds();
    return iq_poll(doomkey, pressed);
}
