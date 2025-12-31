//
#include "isr.h"
//UNT// src/keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H



void init_keyboard(void);
void keyboard_handler_c(registers_t *regs);
char keyboard_get_char(void);


#endif