// src/system.h
#include "stdint.h"
#include "isr.h"
#ifndef __SYSTEM_H
#define __SYSTEM_H

// Объявления функций I/O на Ассемблере
extern unsigned char inb(unsigned short port);
extern void outb(uint16_t port, uint8_t value);

// Объявление функции загрузки IDT
extern void lidt(void *idtr_ptr);
extern void idt_load(void);

#endif