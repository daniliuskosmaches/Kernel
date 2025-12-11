//
// Created by nazar on 09.12.25.
//
// src/vga.c

#include "vga.h"

#include "stdint.h"
#include "stddef.h"
#include "string.h" // Для strlen и других, если они используются

// Параметры VGA
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

// Цвет по умолчанию
static uint8_t terminal_color;

// Позиция курсора
static size_t terminal_row;
static size_t terminal_column;

// Глобальный указатель на VGA-буфер.
// Объявлен как static, чтобы был виден только здесь.co
static uint16_t* vga_buffer;


// ----------------------------------------------------------------------
// Вспомогательные функции
// ----------------------------------------------------------------------

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

// Прокрутка экрана на одну строку
static void terminal_scroll() {
    // ... (код прокрутки)
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index_old = y * VGA_WIDTH + x;
            const size_t index_new = (y - 1) * VGA_WIDTH + x;
            vga_buffer[index_new] = vga_buffer[index_old];
        }
    }
    // ... (очистка последней строки)
    uint16_t blank = vga_entry(' ', terminal_color);
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
    terminal_row = VGA_HEIGHT - 1;
}

// ----------------------------------------------------------------------
// Публичный API VGA
// ----------------------------------------------------------------------

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: Используем локальный указатель 0xB8000
    uint16_t* const local_vga_buffer = (uint16_t*)0xB8000;
    vga_buffer = local_vga_buffer; // Присваиваем глобальному указателю

    uint16_t entry = vga_entry(' ', terminal_color);

    // Очистка экрана
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = entry;
        }
    }
}

// ... (остальные функции)

void terminal_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

void terminal_put_char(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else if (c == '\r') {
        terminal_column = 0;
    } else {
        terminal_put_entry_at(c, terminal_color, terminal_column, terminal_row);
        terminal_column++;
    }

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }
}

void terminal_write_string(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_put_char(data[i]);
    }
}

void terminal_write_hex(uint32_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[11]; // "0x" + 8 hex digits + null
    buffer[0] = '0';
    buffer[1] = 'x';

    for (int i = 7; i >= 0; i--) {
        buffer[2 + i] = hex_chars[n & 0xF];
        n >>= 4;
    }
    buffer[10] = '\0';

    terminal_write_string(buffer);
}

void terminal_backspace(void) {
    if (terminal_column > 0) {
        terminal_column--;
        terminal_put_entry_at(' ', terminal_color, terminal_column, terminal_row);
    }
}

void terminal_clear_screen(void) {
    terminal_initialize();
}
