#include "vga.h"
#include "system.h"
#include "stdint.h"
#include "stddef.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;

// Обновление аппаратного курсора через порты ввода-вывода
void terminal_update_cursor(void) {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Начальная инициализация терминала
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x07; // Серый текст на черном фоне
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
    }
    terminal_update_cursor();
}

void terminal_clear_screen(void) {
    terminal_initialize();
}

// Прокрутка экрана вверх при достижении нижней границы
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

// Базовая функция вывода символа
void terminal_put_char(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    }
    else if (c == '\b') {
        // Логика Backspace: сдвигаем курсор назад, если не в начале строки
        if (terminal_column > 0) {
            terminal_column--;
            // Важно: затираем символ пробелом, иначе буква останется на месте
            vga_buffer[terminal_row * VGA_WIDTH + terminal_column] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
        }
    }
    else {
        // Обычный символ
        vga_buffer[terminal_row * VGA_WIDTH + terminal_column] = (uint16_t)c | (uint16_t)terminal_color << 8;
        terminal_column++;
    }

    // Проверка границ экрана
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

// Вывод чисел в HEX формате (для адресов и отладки)
void terminal_write_hex(uint32_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    terminal_write_string("0x");
    for (int i = 28; i >= 0; i -= 4) {
        terminal_put_char(hex_chars[(n >> i) & 0xF]);
    }
}

// Вывод чисел в десятичном формате
void terminal_write_dec(uint32_t n) {
    if (n == 0) {
        terminal_put_char('0');
        return;
    }
    char buf[10];
    int i = 9;
    while (n > 0) {
        buf[i--] = (n % 10) + '0';
        n /= 10;
    }
    while (++i < 10) {
        terminal_put_char(buf[i]);
    }
}

// Специальная функция для Backspace в Шелле (с защитой промпта "myos> ")
void terminal_backspace() {
    if (terminal_column > 6) {
        terminal_put_char('\b');
    }
}

// Смена цвета (0x0F - белый, 0x04 - красный и т.д.)
void terminal_set_color(uint8_t color) {
    terminal_color = color;
}