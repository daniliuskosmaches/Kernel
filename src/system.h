// src/system.h

#ifndef __SYSTEM_H
#define __SYSTEM_H

// Объявления функций I/O на Ассемблере
extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char data);

// Объявление функции загрузки IDT
extern void lidt(void *idtr_ptr);

#endif