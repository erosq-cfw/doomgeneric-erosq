#ifndef INPUT_MAP_H
#define INPUT_MAP_H
#include <stdint.h>

/* ponytail: hardcode the 3 Linux input-event constants we use instead of
 * pulling in <linux/input-event-codes.h>, which also defines a whole keyboard
 * KEY_* namespace that collides with doomgeneric's doomkeys.h (KEY_ENTER,
 * KEY_TAB, KEY_F1 ...). These are stable kernel UAPI values. */
#ifndef EV_KEY
#define EV_KEY    0x01
#define EV_REL    0x02
#define EV_ABS    0x03
#define REL_X     0x00   /* erosq rotary-encoder axis (DTS linux,axis=<0>) */
#define REL_WHEEL 0x08
#define ABS_HAT0X 0x10   /* BT gamepad d-pad axes: -1/0/+1 */
#define ABS_HAT0Y 0x11
#endif

int map_event(uint16_t type, uint16_t code, int value, unsigned char *doomkey, int *pressed);
#endif
