//
// Created by nazar on 11.12.25.
//

// src/io.h
#ifndef IO_H
#define IO_H

#include <stdint.h>

// Читает байт из порта
extern uint8_t inb(uint16_t port);

// Читает слово (16 бит) из порта
extern uint16_t inw(uint16_t port);

// Записывает байт в порт
extern void outb(uint16_t port, uint8_t data);

// Записывает слово (16 бит) в порт
extern void outw(uint16_t port, uint16_t data);

#endif