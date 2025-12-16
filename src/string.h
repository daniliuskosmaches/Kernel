//
// src/string.h
//

#ifndef STRING_H
#define STRING_H

#include "stddef.h" // Для типа size_t
#include "stdint.h" // Для типов вроде uint8_t

// ============================================================
// ФУНКЦИИ УПРАВЛЕНИЯ ПАМЯТЬЮ
// ============================================================

/**
 * Копирует n байтов из src в dest.
 */
void *memcpy(void *dest, const void *src, size_t n);

/**
 * Копирует n байтов из src в dest. Безопасна при перекрывающихся областях.
 */
void *memmove(void *dest, const void *src, size_t n);

/**
 * Заполняет n байтов памяти по адресу s значением c.
 */
void *memset(void *s, int c, size_t n);


// ============================================================
// ФУНКЦИИ УПРАВЛЕНИЯ СТРОКАМИ
// ============================================================

/**
 * Вычисляет длину строки.
 */
size_t strlen(const char *s);

/**
 * Сравнивает две строки.
 */
int strcmp(const char *s1, const char *s2);

/**
 * Сравнивает n символов двух строк.
 * * ЭТОТ ПРОТОТИП КРИТИЧЕСКИ ВАЖЕН ДЛЯ ИСПРАВЛЕНИЯ ОШИБКИ strncmp
 */
int strncmp(const char *s1, const char *s2, size_t n);

/**
 * Копирует строку src в dest.
 */
char *strcpy(char *dest, const char *src);

/**
 * Добавляет строку src в конец dest.
 */
char *strcat(char *dest, const char *src);

#endif // STRING_H