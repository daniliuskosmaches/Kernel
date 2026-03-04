#include "../include/vga.h"
#include "../include/system.h"
#include "stdint.h"



static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static size_t terminal_limit = 0; // Лимит для Backspace

void terminal_update_cursor(void) {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}


void terminal_set_limit(size_t limit) {
    terminal_limit = limit;
}
void terminal_clear_screen(void) {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
    }
    terminal_row = 0;
    terminal_column = 0;
    terminal_update_cursor();
}
void terminal_set_color(uint8_t color);




void terminal_scroll(void);
void terminal_write_hex(uint32_t n) {
    char hex_str[9];
    for (int i = 0; i < 8; i++) {
        uint8_t digit = (n >> ((7 - i) * 4)) & 0xF;
        hex_str[i] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
    }
    hex_str[8] = '\0';
    terminal_write_string(hex_str);
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x07; // Light gray on black
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
    }
    terminal_update_cursor();
}

void terminal_set_color(uint8_t color) {
    terminal_color = color;
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
    }
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_put_char(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else if (c == '\b') {
        // Защита: не стираем промпт
        if (terminal_column > terminal_limit) {
            terminal_column--;
            vga_buffer[terminal_row * VGA_WIDTH + terminal_column] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
        }
    } else {
        vga_buffer[terminal_row * VGA_WIDTH + terminal_column] = (uint16_t)c | (uint16_t)terminal_color << 8;
        terminal_column++;
    }

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }
    terminal_update_cursor();
}

void terminal_write_string(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_put_char(data[i]);
    }
}