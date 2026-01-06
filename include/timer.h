// src/timer.h
#ifndef TIMER_H
#define TIMER_H

#include "idt.h"
#include "isr.h"

// Глобальный счетчик тиков
extern volatile unsigned long timer_ticks;

// ============================================================
// ПРОТОТИПЫ ФУНКЦИЙ ТАЙМЕРА
// ============================================================

/**
 * Инициализирует PIT (Programmable Interval Timer)
 * @param frequency: Частота в Герцах (рекомендуется 100 Гц)
 */
void time_install(unsigned long frequency);

/**
 * C-обработчик прерывания таймера (IRQ0)
 * Вызывается из isr_handler_c
 */
void timer_handler_c(registers_t *regs);

/**
 * Возвращает количество тиков с момента запуска
 * @return: Количество тиков
 */
unsigned long timer_get_ticks(void);

/**
 * Простая функция задержки
 * @param ticks: Количество тиков для ожидания
 */
void timer_wait(unsigned long ticks);

/**
 * Задержка в миллисекундах
 * @param milliseconds: Количество миллисекунд
 */
void timer_sleep_ms(unsigned long milliseconds);

#endif // TIMER_H