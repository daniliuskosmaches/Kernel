//
// Created by nazar on 09.12.25.
//
// src/string.h
#ifndef STRING_H
#define STRING_H

#include "stddef.h"

void* memset(void* bufptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t count);
int strcmp(const char* str1, const char* str2);
size_t strlen(const char* str);

#endif