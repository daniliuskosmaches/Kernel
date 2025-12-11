// src/isr.h

// src/isr.h

#ifndef ISR_H
#define ISR_H


#include "idt.h" // Теперь registers_t берется отсюда!

// ---------------------------------------------------------------------
// ВНИМАНИЕ: Определение структуры registers_t УДАЛЕНО,
// чтобы избежать конфликта с idt.h!
// ---------------------------------------------------------------------


// ---------------------------------------------------------------------
// Константы Interrupt Request (IRQ) Lines
// ---------------------------------------------------------------------

// Master PIC
#define IRQ0 32  // Timer (PIT)
#define IRQ1 33  // Keyboard (Клавиатура)
#define IRQ2 34
#define IRQ3 35  // COM2
#define IRQ4 36  // COM1
#define IRQ5 37  // LPT2
#define IRQ6 38  // Floppy Disk
#define IRQ7 39  // LPT1

// Slave PIC
#define IRQ8 40  // Real-Time Clock (RTC)
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44 // PS/2 Mouse
#define IRQ13 45 // FPU, Co-processor
#define IRQ14 46 // Primary ATA Hard Disk
#define IRQ15 47 // Secondary ATA Hard Disk

// ---------------------------------------------------------------------
// Прототипы функций
// ---------------------------------------------------------------------

// Объявление глобального массива обработчиков (для использования в других .c файлах)
extern void (*interrupt_handlers[256])(registers_t *regs);

// Главная функция для установки всей IDT
void idt_install(void);

// Функция для установки конкретного обработчика прерывания
void install_interrupt_handler(int irq_number, void (*handler)(registers_t *regs));

// Главный обработчик прерываний на C
void isr_handler_c(registers_t *regs);

#endif