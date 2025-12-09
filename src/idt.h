// src/idt.h

#ifndef __IDT_H
#define __IDT_H

#include "system.h" // Для inb, outb, lidt

// Структура для одного дескриптора в IDT (64-бит / 8 байт)
struct idt_entry {
    unsigned short base_low;    // Младшие 16 бит адреса обработчика (ISR)
    unsigned short selector;    // Сегментный селектор (CS, обычно 0x08)
    unsigned char  zero;        // Всегда 0
    unsigned char  flags;       // Флаги (присутствие, тип гейта и DPL)
    unsigned short base_high;   // Старшие 16 бит адреса обработчика
} __attribute__((packed)); // Важно для выравнивания!

// Структура для указателя IDT (IDTR)
struct idt_ptr {
    unsigned short limit;       // Размер IDT (в байтах)
    unsigned int   base;        // 32-битный адрес начала IDT
} __attribute__((packed));

typedef struct registers{
    unsigned int ds;                 // Сегмент данных
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha сохраняет их
    unsigned int int_no, err_code;   // Номер прерывания и код ошибки
    unsigned int eip, cs, eflags, useresp, ss; // Автоматически сохраняется CPU
}registers_t;
// Объявления функций
extern void idt_install();
extern void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void page_fault_handler_c(registers_t *regs);

#endif