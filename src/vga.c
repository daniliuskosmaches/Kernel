//
// Created by nazar on 09.12.25.
//
// src/vga.c

// Адрес видеопамяти VGA Text Mode
#define VGA_ADDRESS 0xB8000

#include "vga.h"
#include "string.h"

// 1. Используем size_t, чтобы соответствовать объявлению в vga.h
 // Строка 11
size_t vga_row = 0;
size_t vga_col = 0;


// 2. Используем uint16_t* const, чтобы соответствовать объявлению в vga.h
// Переносим константность (const) в объявление, но используем const в определении
// Если VGA_ADDRESS определено как 0xB8000, то:
uint16_t* const vga_buffer = (uint16_t*)VGA_ADDRESS;


void terminal_put_char(char c) {

    if (c == '\n') {
        vga_row++;
        vga_col = 0;
    } else {
        // Создаем символ: ASCII код | (цвет << 8)
        unsigned short entry = c | (0x0F << 8); // 0x0F = Белый на черном

        // Записываем символ в текущую позицию
        vga_buffer[vga_row * 80 + vga_col] = entry;

        vga_col++;
        if (vga_col >= 80) {
            vga_row++;
            vga_col = 0;
        }
    }

    // Простая прокрутка (если строка выходит за пределы 25 строк)
    if (vga_row >= 25) {
        // (Для простоты, здесь должна быть логика прокрутки,
        // пока просто сбросим курсор на начало)
        vga_row = 0;
        vga_col = 0;
    }
}


// src/vga.c (Фрагмент)

void terminal_write_string(const char* data) {
    size_t i = 0;
    while (data[i] != '\0') {
        terminal_put_char(data[i]);
        i++;
    }
}

// src/vga.c (Фрагмент)

void terminal_write_hex(uint32_t n) {
    char *hex = "0123456789ABCDEF";
    char buffer[9]; // 8 символов + \0
    buffer[8] = '\0';

    // Преобразование числа в hex-строку
    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex[n & 0xF]; // Берем младшие 4 бита
        n >>= 4; // Сдвигаем на 4 бита
    }

    // Вывод строки (если вы не хотите выводить ведущие нули,
    // нужно добавить логику пропуска)
    terminal_write_string(buffer);
}

void terminal_initialize(void) {
    // 1. Устанавливаем цвет по умолчанию (например, белый на черном, 0x0F)
    const uint8_t color = 0x0F;

    // 2. Определяем символ, который будем использовать для заполнения (пробел + цвет)
    // Пробел имеет ASCII код 0x20
    const uint16_t entry = (uint16_t)' ' | (color << 8);

    // 3. Заполняем весь экран (80 столбцов * 25 строк = 2000 символов)
    for (size_t i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = entry;
    }

    // 4. Сбрасываем курсор в верхний левый угол
    vga_row = 0;
    vga_col = 0;

    // В идеале здесь также должна быть функция, скрывающая или
    // перемещающая аппаратный курсор VGA, но для начала это достаточно.
}