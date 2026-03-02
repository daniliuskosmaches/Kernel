#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "isr.h"
#include "stdint.h"

// Определение bool если не определен
#ifndef __cplusplus
typedef uint8_t bool;
#define true 1
#define false 0
#endif


void keyboard_handler(registers_t *regs);
char keyboard_get_char(void);
void keyboard_get_line(char* out);
bool keyboard_line_ready(void);

#endif // KEYBOARD_H