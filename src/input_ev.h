#ifndef INPUT_EV_H
#define INPUT_EV_H
/* Reads all /dev/input/event* devices, maps events to doom keys (input_map),
 * and queues them for DG_GetKey to drain. */
void input_open(void);
int  input_poll(unsigned char *doomkey, int *pressed);
#endif
