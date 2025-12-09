// src/vga.h (Только объявления)

#ifndef VGA_H
#define VGA_H

#include "stdint.h"


// Глобальные переменные: используем extern, чтобы указать,
// что они определены где-то в другом файле (.c)
extern size_t vga_row;
extern size_t vga_col;
extern uint16_t* const vga_buffer;
extern uint16_t* const vga_buffer; // Убедитесь, что это определено в .c

// Прототипы функций (без тел!)
void terminal_initialize(void);
void terminal_put_char(char c);
void terminal_write_string(const char* data);
void terminal_write_hex(uint32_t n);
void terminal_clear_screen(void);
void terminal_dump_memory(uint32_t address, size_t size_bytes);

#endif