//
#include "../include/isr.h"
#include <stdbool.h>
//UNT// src/keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H



void keyboard_init(void);
void keyboard_handler(void);

void keyboard_get_line(char* out);
bool keyboard_line_ready(void);

#endif