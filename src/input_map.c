#include "input_map.h"
#include "doomkeys.h"
int map_event(uint16_t type, uint16_t code, int value, unsigned char *k, int *pressed)
{
    if (type == EV_REL && code == REL_WHEEL) {
        *k = value > 0 ? KEY_LEFTARROW : KEY_RIGHTARROW; *pressed = 1; return 1;
    }
    if (type != EV_KEY || value == 2) return 0;  /* ignore autorepeat (value==2) */
    *pressed = value;                            /* 1=down 0=up */
    switch (code) {
        case 165: *k = KEY_UPARROW;    return 1; /* prev  -> forward */
        case 163: *k = KEY_DOWNARROW;  return 1; /* next  -> back    */
        case 115: *k = KEY_STRAFE_L;   return 1; /* vol+  -> strafe L*/
        case 114: *k = KEY_STRAFE_R;   return 1; /* vol-  -> strafe R*/
        case 164: *k = KEY_FIRE;       return 1; /* play  -> fire    */
        case 158: *k = KEY_USE;        return 1; /* back  -> use     */
        case 139: *k = KEY_ESCAPE;     return 1; /* menu  -> esc     */
        default:  return 0;                      /* power(116) etc ignored */
    }
}
