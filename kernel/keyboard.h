#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init(void);
/* Block (halting the CPU) until a character is available, then return it.
 * Ctrl+C is returned as 3 and Ctrl+L as '\f'. */
char keyboard_getchar(void);

#endif
