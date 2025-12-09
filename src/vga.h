//
// Created by nazar on 09.12.25.
//

// src/vga.h
#ifndef VGA_H
#define VGA_H

#include <stdint.h>

// Основные функции терминала
void terminal_initialize(void);
void terminal_put_char(char c);
void terminal_write_string(const char* data);

// Если вы планируете выводить числа (для отладки PMM/VMM)
void terminal_write_dec(uint32_t n);
void terminal_write_hex(uint32_t n);

#endif