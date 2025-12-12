// src/timer.h

#ifndef TIMER_H
#define TIMER_H

#include "idt.h" // Для registers_t
#include "isr.h" // Для IRQ0

// ---------------------------------------------------------------------
// ПРОТОТИПЫ ФУНКЦИЙ ТАЙМЕРА
// ---------------------------------------------------------------------

// Основная функция для инициализации PIT (Programmable Interval Timer)
void time_install(unsigned long freq);

// C-обработчик прерывания таймера (вызывается из isr_handler_c)
void timer_handler_c(registers_t *regs);

#endif