#include <assert.h>
#include "input_queue.h"
#include "input_map.h"   /* EV_KEY/EV_REL/REL_WHEEL */
#include "doomkeys.h"

int main(void)
{
    unsigned char k; int p;

    /* --- wheel: press and release must NOT share a poll cycle --- */
    iq_feed(EV_REL, REL_WHEEL, 1);           /* turn left */
    int press_c1 = 0, release_c1 = 0;
    while (iq_poll(&k, &p)) {                 /* drain cycle 1 (one tic) */
        if (k == KEY_LEFTARROW && p) press_c1 = 1;
        if (k == KEY_LEFTARROW && !p) release_c1 = 1;
    }
    assert(press_c1 && !release_c1);          /* held down through this tic */

    int release_c2 = 0;
    while (iq_poll(&k, &p)) {                 /* drain cycle 2 (next tic) */
        if (k == KEY_LEFTARROW && !p) release_c2 = 1;
    }
    assert(release_c2);                        /* released one tic later */

    /* --- real button down+up in one cycle pass straight through (real HW
     *     timing; only the wheel is deferred) --- */
    iq_feed(EV_KEY, 164, 1);                  /* play=enter down */
    iq_feed(EV_KEY, 164, 0);                  /* enter up */
    int down = 0, up = 0;
    while (iq_poll(&k, &p)) {
        if (k == KEY_ENTER && p) down = 1;
        if (k == KEY_ENTER && !p) up = 1;
    }
    assert(down && up);

    /* power (116) still ignored end-to-end */
    iq_feed(EV_KEY, 116, 1);
    assert(iq_poll(&k, &p) == 0);
    return 0;
}
