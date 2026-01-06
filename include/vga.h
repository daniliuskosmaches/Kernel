// src/vga.h (Только объявления)

#ifndef VGA_H
#define VGA_H

#include "stdint.h"
enum vga_color {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
};

// Глобальные переменные: используем extern, чтобы указать,
// что они определены где-то в другом файле (.c)
extern size_t vga_row;
extern size_t vga_col;
// Убедитесь, что это определено в .c

// Прототипы функций (без тел!)
void terminal_initialize(void);
void terminal_put_char(char c);
void terminal_write_string(const char* data);
void terminal_write_hex(uint32_t n);
void terminal_clear_screen(void);
void terminal_dump_memory(uint32_t address, size_t size_bytes);
void terminal_backspace(void);

#endif