//
// Created by nazar on 10.12.25.
//

#include "stddef.h"
#include "string.h"
#include "../../include/vga.h"
#include "../../include/lib/stdio.h"


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

int strncmp(const char* str1, const char* str2, size_t n) {
    while (n-- && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    if (n == (size_t)-1) return 0; // Если мы сравнили n символов и они все совпали
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}
void *memmove(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dest;
}

int sscanf(const char* str, const char* format, ...) {
    // Это очень упрощенная версия sscanf, которая поддерживает только %x и %zu
    va_list args;
    va_start(args, format);

    size_t i = 0, f = 0;
    while (format[f]) {
        if (format[f] == '%') {
            f++;
            if (format[f] == 'x') {
                uint32_t *out = va_arg(args, uint32_t*);
                *out = 0;
                while (str[i] && ((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'f') || (str[i] >= 'A' && str[i] <= 'F'))) {
                    *out *= 16;
                    if (str[i] >= '0' && str[i] <= '9') {
                        *out += str[i] - '0';
                    } else if (str[i] >= 'a' && str[i] <= 'f') {
                        *out += str[i] - 'a' + 10;
                    } else if (str[i] >= 'A' && str[i] <= 'F') {
                        *out += str[i] - 'A' + 10;
                    }
                    i++;
                }
            } else if (format[f] == 'z' && format[f+1] == 'u') {
                f += 2;
                size_t *out = va_arg(args, size_t*);
                *out = 0;
                while (str[i] && str[i] >= '0' && str[i] <= '9') {
                    *out *= 10;
                    *out += str[i] - '0';
                    i++;
                }
            } else {
                // Unsupported format specifier
                va_end(args);
                return -1;
            }
        } else {
            // Skip non-format characters in the format string
            while (format[f] && format[f] != '%') f++;
        }
    }

    va_end(args);
    return 0; // Success
}


size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

// src/string.c (Добавьте это в конец файла)
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    // Если src короче n, заполняем остаток нулями
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

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