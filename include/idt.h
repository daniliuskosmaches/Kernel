// src/idt.h (ИСПРАВЛЕННО, ВКЛЮЧАЕТ СТРУКТУРЫ)

#ifndef IDT_H
#define IDT_H
#define IRQ_MASTER_OFFSET 0x20 // Начало IRQ Master (IRQ0 = 0x20)
#define IRQ_SLAVE_OFFSET  0x28 // Начало IRQ Slave (IRQ8 = 0x28)

// Определения для IRQ0 (Таймер) и IRQ1 (Клавиатура)
#define IRQ0 (IRQ_MASTER_OFFSET + 0) // Таймер
#define IRQ1 (IRQ_MASTER_OFFSET + 1) // Клавиатура <--- ЭТО РЕШАЕТ ПРОБЛЕМУ 'IRQ1'


#include <stdint.h>
#include "isr.h" // Импортирует registers_t

// --- IDT Structures (DEFINITIONS) ---
// Эти структуры критически важны и должны быть в заголовке
// чтобы idt.c мог определить глобальные массивы.
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


// --- ПУБЛИЧНЫЕ ПРОТОТИПЫ ---

extern void idt_install(void);
extern void register_interrupt_handler(uint8_t n, void (*handler)(registers_t *regs));
extern void pic_enable_irq(uint8_t irq);
extern void init_keyboard(void);



#endif // IDT_H