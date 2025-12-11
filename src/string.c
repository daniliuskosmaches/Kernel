//
// Created by nazar on 10.12.25.
//
#include "stddef.h"
#include "string.h"
#include "vga.h"

void *memset(void *s, int c, size_t n) {
    unsigned char *ptr = s;
    while (n--) {
        *ptr++ = (unsigned char)c;
    }
    return s;
}
void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

// src/string.c (Добавьте это в конец файла)


// ------------------------------------------------------------------
// Реализация функции просмотра памяти (memspy)
// Выводит блок памяти в шестнадцатеричном формате
// ------------------------------------------------------------------

void terminal_dump_memory(uint32_t address, size_t size_bytes) {
    uint8_t *ptr = (uint8_t *)address;
    size_t i;

    // Если размер 0, то ничего не делаем
    if (size_bytes == 0) return;

    terminal_write_string("--- Memory Dump at 0x");
    terminal_write_hex(address);
    terminal_write_string(" (");
    terminal_write_hex(size_bytes);
    terminal_write_string(" bytes) ---\n");

    for (i = 0; i < size_bytes; i++) {
        // Выводим адрес начала строки (каждые 16 байт)
        if (i % 16 == 0) {
            if (i != 0) {
                // Перевод строки и ascii-представление предыдущей строки
                terminal_write_string("  ");
                // Здесь можно добавить цикл для вывода ASCII-эквивалентов
                for (size_t j = i - 16; j < i; j++) {
                    char c = ptr[j];
                    // Выводим точку, если символ непечатаемый, или сам символ
                    terminal_put_char((c >= ' ' && c <= '~') ? c : '.');
                }
                terminal_write_string("\n");
            }

            // Выводим адрес текущей строки
            terminal_write_hex((uint32_t)ptr + i);
            terminal_write_string(": ");
        }

        // Выводим байт в шестнадцатеричном формате
        uint8_t byte = ptr[i];
        if (byte < 0x10) {
            terminal_write_string("0"); // Добавляем ведущий ноль
        }
        terminal_write_hex(byte);
        terminal_write_string(" ");
    }

    // Выводим последний неполный хвост (пробелы и ASCII)
    size_t remainder = size_bytes % 16;
    if (remainder != 0) {
        // Добавляем пробелы, чтобы выровнять вывод ASCII
        for (size_t k = 0; k < 16 - remainder; k++) {
            terminal_write_string("   ");
        }
        terminal_write_string("  ");

        // Вывод ASCII для последней строки
        for (size_t j = size_bytes - remainder; j < size_bytes; j++) {
            char c = ptr[j];
            terminal_put_char((c >= ' ' && c <= '~') ? c : '.');
        }
    }
    terminal_write_string("\n");
}