// src/stdint.h

#ifndef STDINT_H
#define STDINT_H

// Мы используем встроенные типы GCC, чтобы избежать конфликта с stdint-gcc.h
// Эти типы обычно доступны, даже если стандартный stdint.h отключен.

// Типы фиксированного размера
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
// Заменяем 'unsigned int' на 'unsigned long' или 'int' (зависит от вашего тулчейна),
// но лучше использовать встроенные типы GCC:
typedef __UINT32_TYPE__ uint32_t;
typedef __INT32_TYPE__  int32_t;

// Если __UINT32_TYPE__ не работает, попробуйте:
// typedef unsigned int uint32_t;
// typedef signed int int32_t;

typedef unsigned long long uint64_t;
typedef signed long long   int64_t;

typedef __SIZE_TYPE__ size_t; // Тип для размеров (обычно unsigned int или unsigned long)

#endif