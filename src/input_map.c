#include "input_map.h"
#include "doomkeys.h"

/* BT gamepad (8BitDo, HoG/uhid) d-pad arrives as EV_ABS hat axes: -1/0/+1.
 * map_event returns one key per event, so remember which key each axis is
 * holding and emit its release when the axis returns to 0.
 * ponytail: a direct -1 -> +1 flip (no 0 between) would leak a held key; pads
 * always pass through 0 on release, so not handled. */
static unsigned char hat_held[2];   /* [0]=HAT0X, [1]=HAT0Y; 0 = none */
static unsigned char stick_held[2]; /* [0]=ABS_X, [1]=ABS_Y; 0 = none */

int map_event(uint16_t type, uint16_t code, int value, unsigned char *k, int *pressed)
{
    /* The scroll wheel is a rotary-encoder on REL_X (DTS linux,axis=<0>), NOT
     * REL_WHEEL. It reports +1 CW / -1 CCW. Emit turn keys here; DG_GetKey
     * remaps them to UP/DOWN while a menu is open. */
    if (type == EV_REL && code == REL_X) {
        *k = value > 0 ? KEY_RIGHTARROW : KEY_LEFTARROW; *pressed = 1; return 1;
    }
    if (type == EV_ABS && (code == ABS_HAT0X || code == ABS_HAT0Y)) {
        int ax = (code == ABS_HAT0Y);
        if (value == 0) {
            if (!hat_held[ax]) return 0;
            *k = hat_held[ax]; hat_held[ax] = 0; *pressed = 0; return 1;
        }
        if (ax) *k = value < 0 ? KEY_UPARROW   : KEY_DOWNARROW;  /* fwd/back */
        else    *k = value < 0 ? KEY_LEFTARROW : KEY_RIGHTARROW; /* turn     */
        hat_held[ax] = *k; *pressed = 1; return 1;
    }
    /* Left analog stick (ABS_X/ABS_Y, 0..255, centered ~127): treat as the
     * d-pad with a deadzone, same held-key release tracking as the hat. A
     * direction flip without passing centre emits the release first; the
     * stick streams, so the new press lands on the next sample. */
    if (type == EV_ABS && (code == ABS_X || code == ABS_Y)) {
        int ax = (code == ABS_Y);
        unsigned char want = 0;
        if (value < 96)       want = ax ? KEY_UPARROW   : KEY_LEFTARROW;
        else if (value > 160) want = ax ? KEY_DOWNARROW : KEY_RIGHTARROW;
        if (want == stick_held[ax]) return 0;
        if (want == 0 || stick_held[ax] != 0) {
            *k = stick_held[ax]; stick_held[ax] = 0; *pressed = 0; return 1;
        }
        stick_held[ax] = want; *k = want; *pressed = 1; return 1;
    }
    if (type != EV_KEY || value == 2) return 0;  /* ignore autorepeat (value==2) */
    *pressed = value;                            /* 1=down 0=up */
    switch (code) {
        case 165: *k = KEY_UPARROW;    return 1; /* prev  -> forward */
        case 163: *k = KEY_DOWNARROW;  return 1; /* next  -> back    */
        case 115: *k = KEY_STRAFE_L;   return 1; /* vol+  -> strafe L*/
        case 114: *k = KEY_STRAFE_R;   return 1; /* vol-  -> strafe R*/
        case 164: *k = KEY_ENTER;      return 1; /* play  -> enter (menu select; fires in-game via key_fire=ENTER; confirms quit) */
        case 158: *k = KEY_USE;        return 1; /* back  -> use     */
        case 139: *k = KEY_ESCAPE;     return 1; /* menu  -> esc     */
        /* BT gamepad buttons (evdev BTN_* codes) */
        case 304: *k = KEY_ENTER;      return 1; /* A  -> fire/select */
        case 305: *k = KEY_USE;        return 1; /* B  -> use         */
        case 307: *k = KEY_RSHIFT;     return 1; /* X  -> run         */
        case 310: *k = KEY_STRAFE_L;   return 1; /* L1 -> strafe L    */
        case 311: *k = KEY_STRAFE_R;   return 1; /* R1 -> strafe R    */
        case 314: *k = KEY_TAB;        return 1; /* select -> automap */
        case 315: *k = KEY_ESCAPE;     return 1; /* start  -> menu    */
        case 308: *k = KEY_RALT;       return 1; /* Y  -> strafe hold (key_strafe) */
        case 312: *k = 0xb0;           return 1; /* L2 -> prev weapon (key_prevweapon) */
        case 313: *k = 0xb1;           return 1; /* R2 -> next weapon (key_nextweapon) */
        default:  return 0;                      /* power(116) etc ignored */
    }
}
