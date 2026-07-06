#include <assert.h>
#include "input_map.h"   /* EV_KEY=1, EV_REL=2, REL_WHEEL=8 defined here */
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
    assert(map_event(EV_REL, REL_WHEEL,  1, &k, &p) && k == KEY_LEFTARROW  && p == 1);
    assert(map_event(EV_REL, REL_WHEEL, -1, &k, &p) && k == KEY_RIGHTARROW && p == 1);
    return 0;
}
