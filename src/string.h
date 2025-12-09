//
// Created by nazar on 09.12.25.
//
// src/string.h
#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void* memset(void* bufptr, int value, size_t size) {
    // Приводим указатель к указателю на байт (unsigned char) для побайтовой записи
    unsigned char* buf = (unsigned char*)bufptr;

    // Итерируемся по всем 'size' байтам
    for (size_t i = 0; i < size; i++) {
        // Присваиваем каждому байту значение (приведенное к unsigned char)
        buf[i] = (unsigned char)value;
    }

    return bufptr;
}
#endif