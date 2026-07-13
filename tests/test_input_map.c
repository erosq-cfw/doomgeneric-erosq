#include <assert.h>
#include "input_map.h"   /* EV_KEY=1, EV_REL=2, REL_X=0, REL_WHEEL=8 defined here */
#include "doomkeys.h"
static unsigned char k; static int p;
int main(void)
{
    assert(map_event(EV_KEY, 165, 1, &k, &p) && k == KEY_UPARROW    && p == 1); /* prev=fwd */
    assert(map_event(EV_KEY, 163, 1, &k, &p) && k == KEY_DOWNARROW  && p == 1); /* next=back */
    assert(map_event(EV_KEY, 115, 0, &k, &p) && k == KEY_STRAFE_L   && p == 0); /* vol+ release */
    assert(map_event(EV_KEY, 114, 1, &k, &p) && k == KEY_STRAFE_R   && p == 1); /* vol- */
    assert(map_event(EV_KEY, 164, 1, &k, &p) && k == KEY_ENTER      && p == 1); /* play=enter */
    assert(map_event(EV_KEY, 158, 1, &k, &p) && k == KEY_USE        && p == 1); /* back=use */
    assert(map_event(EV_KEY, 139, 1, &k, &p) && k == KEY_ESCAPE     && p == 1); /* menu=esc */
    assert(map_event(EV_KEY, 116, 1, &k, &p) == 0);                             /* power ignored */
    /* Wheel = rotary-encoder on REL_X (verified on HW: /proc REL=1, live evtest
     * code=0). REL_WHEEL is NOT what the device emits. */
    assert(map_event(EV_REL, REL_X,  1, &k, &p) && k == KEY_RIGHTARROW && p == 1);
    assert(map_event(EV_REL, REL_X, -1, &k, &p) && k == KEY_LEFTARROW  && p == 1);
    assert(map_event(EV_REL, REL_WHEEL, 1, &k, &p) == 0);  /* real mice ignored; only the encoder wheel */
    /* BT gamepad (8BitDo over HoG/uhid, HW-captured codes 2026-07-13) */
    assert(map_event(EV_KEY, 304, 1, &k, &p) && k == KEY_ENTER      && p == 1); /* A=fire */
    assert(map_event(EV_KEY, 305, 1, &k, &p) && k == KEY_USE        && p == 1); /* B=use */
    assert(map_event(EV_KEY, 307, 1, &k, &p) && k == KEY_RSHIFT     && p == 1); /* X=run */
    assert(map_event(EV_KEY, 310, 1, &k, &p) && k == KEY_STRAFE_L   && p == 1); /* L1 */
    assert(map_event(EV_KEY, 311, 1, &k, &p) && k == KEY_STRAFE_R   && p == 1); /* R1 */
    assert(map_event(EV_KEY, 314, 1, &k, &p) && k == KEY_TAB        && p == 1); /* select=automap */
    assert(map_event(EV_KEY, 315, 1, &k, &p) && k == KEY_ESCAPE     && p == 1); /* start=menu */
    /* d-pad hat: press emits direction, return-to-0 emits that key's release */
    assert(map_event(EV_ABS, ABS_HAT0Y, -1, &k, &p) && k == KEY_UPARROW    && p == 1);
    assert(map_event(EV_ABS, ABS_HAT0Y,  0, &k, &p) && k == KEY_UPARROW    && p == 0);
    assert(map_event(EV_ABS, ABS_HAT0X,  1, &k, &p) && k == KEY_RIGHTARROW && p == 1);
    assert(map_event(EV_ABS, ABS_HAT0X,  0, &k, &p) && k == KEY_RIGHTARROW && p == 0);
    assert(map_event(EV_ABS, ABS_HAT0X,  0, &k, &p) == 0);  /* idle 0 after release: no event */
    /* independent axes: X press, Y press, X release keeps Y held */
    assert(map_event(EV_ABS, ABS_HAT0X, -1, &k, &p) && k == KEY_LEFTARROW  && p == 1);
    assert(map_event(EV_ABS, ABS_HAT0Y,  1, &k, &p) && k == KEY_DOWNARROW  && p == 1);
    assert(map_event(EV_ABS, ABS_HAT0X,  0, &k, &p) && k == KEY_LEFTARROW  && p == 0);
    assert(map_event(EV_ABS, ABS_HAT0Y,  0, &k, &p) && k == KEY_DOWNARROW  && p == 0);
    return 0;
}
